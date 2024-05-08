/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "wakeup_engine_impl.h"
#include "audio_system_manager.h"
#include "v1_2/intell_voice_engine_types.h"
#include "adapter_callback_service.h"
#include "intell_voice_log.h"
#include "history_info_mgr.h"
#include "intell_voice_util.h"
#include "intell_voice_service_manager.h"
#include "trigger_manager.h"
#include "engine_host_manager.h"

#define LOG_TAG "WakeupEngineImpl"

using namespace OHOS::AudioStandard;
using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::IntellVoiceTrigger;

namespace OHOS {
namespace IntellVoiceEngine {
static constexpr uint32_t MIN_BUFFER_SIZE = 640;
static constexpr uint32_t INTERVAL = 100;
static constexpr int32_t CHANNEL_CNT = 1;
static constexpr int32_t BITS_PER_SAMPLE = 16;
static constexpr int32_t SAMPLE_RATE = 16000;
static const std::string WAKEUP_SOURCE_CHANNEL = "wakeup_source_channel";
static constexpr std::string_view DEFAULT_WAKEUP_PHRASE = "\xE5\xB0\x8F\xE8\x89\xBA\xE5\xB0\x8F\xE8\x89\xBA";
static constexpr int64_t RECOGNIZING_TIMEOUT_US = 10 * 1000 * 1000; //10s
static constexpr int64_t RECOGNIZE_COMPLETE_TIMEOUT_US = 1 * 1000; //1ms
static constexpr int64_t READ_CAPTURER_TIMEOUT_US = 10 * 1000 * 1000; //10s

WakeupEngineImpl::WakeupEngineImpl() : ModuleStates(State(IDLE), "WakeupEngineImpl", "WakeupThread")
{
    InitStates();
    capturerOptions_.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_16000;
    capturerOptions_.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions_.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions_.capturerInfo.sourceType = SourceType::SOURCE_TYPE_WAKEUP;
    capturerOptions_.capturerInfo.capturerFlags = 0;
    adapterListener_ = std::make_shared<WakeupAdapterListener>(
        std::bind(&WakeupEngineImpl::OnWakeupEvent, this, std::placeholders::_1, std::placeholders::_2));
    if (adapterListener_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapterListener_ is nullptr");
    }
}

WakeupEngineImpl::~WakeupEngineImpl()
{
}

bool WakeupEngineImpl::InitStates()
{
    for (int i = IDLE; i <= READ_CAPTURER; i++) {
        ForState(State(i))
            .ACT(SET_LISTENER, HandleSetListener)
            .ACT(SET_PARAM, HandleSetParam)
            .ACT(GET_PARAM, HandleGetParam);
    }

    ForState(IDLE)
        .ACT(INIT, HandleInit)
        .ACT(RESET_ADAPTER, HandleResetAdapter);

    ForState(INITIALIZING)
        .ACT(INIT_DONE, HandleInitDone);

    ForState(INITIALIZED)
        .ACT(START_RECOGNIZE, HandleStart);

    ForState(RECOGNIZING)
        .WaitUntil(RECOGNIZING_TIMEOUT,
            std::bind(&WakeupEngineImpl::HandleRecognizingTimeout, this, std::placeholders::_1, std::placeholders::_2),
            RECOGNIZING_TIMEOUT_US)
        .ACT(STOP_RECOGNIZE, HandleStop)
        .ACT(RECOGNIZE_COMPLETE, HandleRecognizeComplete);

    ForState(RECOGNIZED)
        .WaitUntil(RECOGNIZE_COMPLETE_TIMEOUT,
            std::bind(&WakeupEngineImpl::HandleStopCapturer, this, std::placeholders::_1, std::placeholders::_2),
            RECOGNIZE_COMPLETE_TIMEOUT_US)
        .ACT(GET_WAKEUP_PCM, HandleGetWakeupPcm)
        .ACT(START_CAPTURER, HandleStartCapturer)
        .ACT(RECONFIRM_RECOGNITION_COMPLETE, HandleReconfirmRecognitionComplete);

    ForState(READ_CAPTURER)
        .WaitUntil(READ_CAPTURER_TIMEOUT,
            std::bind(&WakeupEngineImpl::HandleStopCapturer, this, std::placeholders::_1, std::placeholders::_2),
            READ_CAPTURER_TIMEOUT_US)
        .ACT(READ, HandleRead)
        .ACT(STOP_CAPTURER, HandleStopCapturer)
        .ACT(RECONFIRM_RECOGNITION_COMPLETE, HandleReconfirmRecognitionComplete);

    FromState(INITIALIZING, READ_CAPTURER)
        .ACT(RELEASE_ADAPTER, HandleRelease)
        .ACT(RELEASE, HandleRelease);

    return IsStatesInitSucc();
}

int32_t WakeupEngineImpl::Handle(const StateMsg &msg)
{
    if (!IsStatesInitSucc()) {
        INTELL_VOICE_LOG_ERROR("failed to init state");
        return -1;
    }

    return ModuleStates::HandleMsg(msg);
}

OHOS::AudioStandard::AudioChannel WakeupEngineImpl::GetWakeupSourceChannel()
{
    auto triggerMgr = TriggerManager::GetInstance();
    if (triggerMgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger manager is nullptr");
        return AudioChannel::MONO;
    }

    std::string channel = triggerMgr->GetParameter(WAKEUP_SOURCE_CHANNEL);
    if (channel == "") {
        INTELL_VOICE_LOG_INFO("no channle info");
        return AudioChannel::MONO;
    }

    auto ret = static_cast<OHOS::AudioStandard::AudioChannel>(std::stoi(channel));
    if (ret > AudioChannel::CHANNEL_4) {
        INTELL_VOICE_LOG_INFO("invalid channel, ret:%{public}d", ret);
        return AudioChannel::MONO;
    }
    INTELL_VOICE_LOG_INFO("channle:%{public}d", static_cast<int32_t>(ret));
    return ret;
}

bool WakeupEngineImpl::SetCallbackInner()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return false;
    }

    if (adapterListener_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapterListener_ is nullptr");
        return false;
    }

    callback_ = sptr<IIntellVoiceEngineCallback>(new (std::nothrow) AdapterCallbackService(adapterListener_));
    if (callback_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("callback_ is nullptr");
        return false;
    }

    adapter_->SetCallback(callback_);
    return true;
}

int32_t WakeupEngineImpl::AttachInner(const IntellVoiceEngineInfo &info)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }

    isPcmFromExternal_ = info.isPcmFromExternal;

    IntellVoiceEngineAdapterInfo adapterInfo = {
        .wakeupPhrase = info.wakeupPhrase,
        .minBufSize = info.minBufSize,
        .sampleChannels = info.sampleChannels,
        .bitsPerSample = info.bitsPerSample,
        .sampleRate = info.sampleRate,
    };
    return adapter_->Attach(adapterInfo);
}

bool WakeupEngineImpl::CreateWakeupSourceStopCallback()
{
    if (wakeupSourceStopCallback_ != nullptr) {
        INTELL_VOICE_LOG_INFO("wakeup close cb is already created");
        return true;
    }

    auto audioSystemManager = AudioSystemManager::GetInstance();
    if (audioSystemManager == nullptr) {
        INTELL_VOICE_LOG_ERROR("audioSystemManager is nullptr");
        return false;
    }

    wakeupSourceStopCallback_ = std::make_shared<WakeupSourceStopCallback>();
    if (wakeupSourceStopCallback_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("wakeup source stop callback is nullptr");
        return false;
    }

    audioSystemManager->SetWakeUpSourceCloseCallback(wakeupSourceStopCallback_);
    return true;
}

void WakeupEngineImpl::DestroyWakeupSourceStopCallback()
{
    if (wakeupSourceStopCallback_ == nullptr) {
        INTELL_VOICE_LOG_INFO("wakeup close cb is already destroyed");
        return;
    }

    auto audioSystemManager = AudioSystemManager::GetInstance();
    if (audioSystemManager == nullptr) {
        INTELL_VOICE_LOG_ERROR("audioSystemManager is nullptr");
        return;
    }

    audioSystemManager->SetWakeUpSourceCloseCallback(nullptr);
    wakeupSourceStopCallback_ = nullptr;
}

bool WakeupEngineImpl::StartAudioSource()
{
    capturerOptions_.streamInfo.channels = GetWakeupSourceChannel();
    WakeupSourceProcess::Init(capturerOptions_.streamInfo.channels);
    auto listener = std::make_unique<AudioSourceListener>(
        [&](uint8_t *buffer, uint32_t size, bool isEnd) {
            std::vector<std::vector<uint8_t>> audioData;
            auto ret = IntellVoiceUtil::DeinterleaveAudioData(reinterpret_cast<int16_t *>(buffer),
                size / sizeof(int16_t), static_cast<int32_t>(capturerOptions_.streamInfo.channels), audioData);
            if ((!ret) || ((audioData.size() != static_cast<uint32_t>(capturerOptions_.streamInfo.channels))) ||
                (channelId_ >= audioData.size())) {
                INTELL_VOICE_LOG_ERROR("failed to deinterleave, ret:%{public}d, id:%{public}d", ret, channelId_);
                return;
            }
            if ((adapter_ != nullptr) && !isEnd) {
                adapter_->WriteAudio(audioData[channelId_]);
            }
            WakeupSourceProcess::Write(audioData);
        },
        [&]() {
            INTELL_VOICE_LOG_INFO("end of pcm");
            if (adapter_ != nullptr) {
                adapter_->SetParameter("end_of_pcm=true");
            }
        });
    if (listener == nullptr) {
        INTELL_VOICE_LOG_ERROR("create listener failed");
        return false;
    }

    audioSource_ = std::make_unique<AudioSource>(MIN_BUFFER_SIZE * static_cast<uint32_t>(
        capturerOptions_.streamInfo.channels), INTERVAL, std::move(listener), capturerOptions_);
    if (audioSource_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("create audio source failed");
        return false;
    }

    if (!audioSource_->Start()) {
        INTELL_VOICE_LOG_ERROR("start capturer failed");
        audioSource_ = nullptr;
        return false;
    }

    return true;
}

void WakeupEngineImpl::StopAudioSource()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (audioSource_ == nullptr) {
        INTELL_VOICE_LOG_INFO("audio source is nullptr, no need to stop");
        return;
    }

    audioSource_->Stop();
    audioSource_ = nullptr;
    WakeupSourceProcess::Release();
}

void WakeupEngineImpl::OnWakeupEvent(int32_t msgId, int32_t result)
{
    if (msgId == INTELL_VOICE_ENGINE_MSG_INIT_DONE) {
        std::thread(&WakeupEngineImpl::OnInitDone, this, result).detach();
    } else if (msgId == INTELL_VOICE_ENGINE_MSG_RECOGNIZE_COMPLETE) {
        std::thread(&WakeupEngineImpl::OnWakeupRecognition, this, result).detach();
    } else if (msgId == static_cast<OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineMessageType>(
        HDI::IntelligentVoice::Engine::V1_2::INTELL_VOICE_ENGINE_MSG_RECONFIRM_RECOGNITION_COMPLETE)) {
        std::thread(&WakeupEngineImpl::OnWakeupKws3Recognition, this, result).detach();
    } else {
    }
}

void WakeupEngineImpl::OnInitDone(int32_t result)
{
    INTELL_VOICE_LOG_INFO("on Init Done, result:%{public}d", result);
    StateMsg msg(INIT_DONE, &result, sizeof(int32_t));
    Handle(msg);
}

void WakeupEngineImpl::OnWakeupRecognition(int32_t result)
{
    INTELL_VOICE_LOG_INFO("on wakeup recognition, result:%{public}d", result);
    StateMsg msg(RECOGNIZE_COMPLETE, &result, sizeof(int32_t));
    Handle(msg);
}

void WakeupEngineImpl::OnWakeupKws3Recognition(int32_t result)
{
    INTELL_VOICE_LOG_INFO("on wakeup kws3 recognition, result:%{public}d", result);
    StateMsg msg(RECONFIRM_RECOGNITION_COMPLETE, &result, sizeof(int32_t));
    Handle(msg);
}

int32_t WakeupEngineImpl::HandleSetParam(const StateMsg &msg, State & /* nextState */)
{
    StringParam *param = reinterpret_cast<StringParam *>(msg.inMsg);
    if (param == nullptr) {
        INTELL_VOICE_LOG_INFO("param is nullptr");
        return -1;
    }

    return EngineUtil::SetParameter(param->strParam);
}

int32_t WakeupEngineImpl::HandleGetParam(const StateMsg &msg, State & /* nextState */)
{
    StringParam *key = reinterpret_cast<StringParam *>(msg.inMsg);
    StringParam *value = reinterpret_cast<StringParam *>(msg.inMsg);
    if ((key == nullptr) || (value == nullptr)) {
        INTELL_VOICE_LOG_INFO("key or value is nullptr");
        return -1;
    }

    value->strParam = EngineUtil::GetParameter(key->strParam);
    return 0;
}

int32_t WakeupEngineImpl::HandleInit(const StateMsg & /* msg */, State &nextState)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (!EngineUtil::CreateAdapterInner(EngineHostManager::GetInstance(), WAKEUP_ADAPTER_TYPE)) {
        INTELL_VOICE_LOG_ERROR("failed to create adapter");
        return -1;
    }

    if (!SetCallbackInner()) {
        INTELL_VOICE_LOG_ERROR("failed to set callback");
        return -1;
    }
    UpdateDspModel();
    EngineUtil::SetLanguage();
    EngineUtil::SetArea();
    SetDspFeatures();

    IntellVoiceEngineInfo info = {
        .wakeupPhrase = GetWakeupPhrase(),
        .isPcmFromExternal = false,
        .minBufSize = MIN_BUFFER_SIZE,
        .sampleChannels = CHANNEL_CNT,
        .bitsPerSample = BITS_PER_SAMPLE,
        .sampleRate = SAMPLE_RATE,
    };
    if (AttachInner(info) != 0) {
        INTELL_VOICE_LOG_ERROR("failed to attach");
        return -1;
    }

    nextState = State(INITIALIZING);
    return 0;
}

int32_t WakeupEngineImpl::HandleInitDone(const StateMsg &msg, State &nextState)
{
    INTELL_VOICE_LOG_INFO("enter");
    int32_t *result = reinterpret_cast<int32_t *>(msg.inMsg);
    if ((result == nullptr) || (*result != 0)) {
        INTELL_VOICE_LOG_ERROR("init done failed");
        return -1;
    }

    nextState = State(INITIALIZED);
    return 0;
}

int32_t WakeupEngineImpl::HandleSetListener(const StateMsg &msg, State & /* nextState */)
{
    SetListenerMsg *listenerMsg = reinterpret_cast<SetListenerMsg *>(msg.inMsg);
    if (listenerMsg == nullptr || adapterListener_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("listenerMsg or adapter listener is nullptr");
        return -1;
    }

    adapterListener_->SetCallback(listenerMsg->callback);
    return 0;
}

int32_t WakeupEngineImpl::HandleStart(const StateMsg &msg, State &nextState)
{
    int32_t *msgBody = reinterpret_cast<int32_t *>(msg.inMsg);
    if (msgBody == nullptr) {
        INTELL_VOICE_LOG_ERROR("msgBody is nullptr");
        return -1;
    }

    channelId_ = ((*msgBody == PROXIMAL_WAKEUP_MODEL_UUID) ? CHANNEL_ID_1 : CHANNEL_ID_0);
    INTELL_VOICE_LOG_INFO("enter, channel id is %{public}d", channelId_);

    EngineUtil::SetParameter("VprTrdType=0;WakeupScene=0");

    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }

    StartInfo info = {
        .isLast = true,
    };
    if (adapter_->Start(info)) {
        INTELL_VOICE_LOG_ERROR("start adapter failed");
        return -1;
    }

    if (!CreateWakeupSourceStopCallback()) {
        INTELL_VOICE_LOG_ERROR("create wakeup source stop callback failed");
        adapter_->Stop();
        return -1;
    }

    if (!StartAudioSource()) {
        INTELL_VOICE_LOG_ERROR("start audio source failed");
        adapter_->Stop();
        return -1;
    }

    INTELL_VOICE_LOG_INFO("exit");
    nextState = State(RECOGNIZING);
    return 0;
}

int32_t WakeupEngineImpl::HandleStop(const StateMsg & /* msg */, State &nextState)
{
    StopAudioSource();
    EngineUtil::Stop();
    nextState = State(INITIALIZED);
    return 0;
}

int32_t WakeupEngineImpl::HandleRecognizeComplete(const StateMsg &msg, State &nextState)
{
    int32_t *result = reinterpret_cast<int32_t *>(msg.inMsg);
    if (result == nullptr) {
        INTELL_VOICE_LOG_ERROR("result is nullptr");
        return -1;
    }
    if (*result != 0) {
        INTELL_VOICE_LOG_ERROR("wakeup failed");
        StopAudioSource();
        EngineUtil::Stop();
        nextState = State(INITIALIZED);
    } else {
        nextState = State(RECOGNIZED);
    }
    return 0;
}

int32_t WakeupEngineImpl::HandleReconfirmRecognitionComplete(const StateMsg &msg, State &nextState)
{
    EngineUtil::Stop();
    return 0;
}

int32_t WakeupEngineImpl::HandleStartCapturer(const StateMsg &msg, State &nextState)
{
    INTELL_VOICE_LOG_INFO("enter");
    int32_t *msgBody = reinterpret_cast<int32_t *>(msg.inMsg);
    if (msgBody == nullptr) {
        INTELL_VOICE_LOG_ERROR("msgBody is nullptr");
        return -1;
    }
    channels_ = *msgBody;
    nextState = State(READ_CAPTURER);
    return 0;
}

int32_t WakeupEngineImpl::HandleRead(const StateMsg &msg, State & /* nextState */)
{
    CapturerData *capturerData = reinterpret_cast<CapturerData *>(msg.outMsg);
    if (capturerData == nullptr) {
        INTELL_VOICE_LOG_ERROR("capturer data is nullptr");
        return -1;
    }

    auto ret = WakeupSourceProcess::Read(capturerData->data, channels_);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("read capturer data failed");
        return ret;
    }

    ResetTimerDelay();
    return 0;
}

int32_t WakeupEngineImpl::HandleStopCapturer(const StateMsg & /* msg */, State &nextState)
{
    INTELL_VOICE_LOG_INFO("enter");
    StopAudioSource();
    EngineUtil::Stop();
    nextState = State(INITIALIZED);
    return 0;
}

int32_t WakeupEngineImpl::HandleRecognizingTimeout(const StateMsg & /* msg */, State &nextState)
{
    INTELL_VOICE_LOG_INFO("enter");
    StopAudioSource();
    EngineUtil::Stop();
    nextState = State(INITIALIZED);
    return 0;
}

int32_t WakeupEngineImpl::HandleGetWakeupPcm(const StateMsg &msg, State & /* nextState */)
{
    CapturerData *capturerData = reinterpret_cast<CapturerData *>(msg.outMsg);
    if (capturerData == nullptr) {
        INTELL_VOICE_LOG_ERROR("capturer data is nullptr");
        return -1;
    }
    auto ret = EngineUtil::GetWakeupPcm(capturerData->data);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("get wakeup pcm failed");
        return ret;
    }
    return 0;
}

int32_t WakeupEngineImpl::HandleResetAdapter(const StateMsg & /* msg */, State &nextState)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (!EngineUtil::CreateAdapterInner(EngineHostManager::GetInstance(), WAKEUP_ADAPTER_TYPE)) {
        INTELL_VOICE_LOG_ERROR("failed to create adapter");
        return -1;
    }
    adapter_->SetCallback(callback_);
    UpdateDspModel();
    EngineUtil::SetLanguage();
    EngineUtil::SetArea();
    SetDspFeatures();

    IntellVoiceEngineAdapterInfo adapterInfo = {
        .wakeupPhrase = GetWakeupPhrase(),
        .minBufSize = MIN_BUFFER_SIZE,
        .sampleChannels = CHANNEL_CNT,
        .bitsPerSample = BITS_PER_SAMPLE,
        .sampleRate = SAMPLE_RATE,
    };
    if (adapter_->Attach(adapterInfo) != 0) {
        INTELL_VOICE_LOG_ERROR("failed to attach");
        EngineUtil::ReleaseAdapterInner(EngineHostManager::GetInstance());
        return -1;
    }

    nextState = State(INITIALIZING);
    return 0;
}

int32_t WakeupEngineImpl::HandleRelease(const StateMsg & /* msg */, State &nextState)
{
    DestroyWakeupSourceStopCallback();
    StopAudioSource();
    if (adapter_ != nullptr) {
        adapter_->Detach();
        ReleaseAdapterInner(EngineHostManager::GetInstance());
    }
    nextState = State(IDLE);
    return 0;
}

void WakeupEngineImpl::UpdateDspModel()
{
    auto &mgr = IntellVoiceServiceManager::GetInstance();
    if (mgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("mgr is nullptr");
        return;
    }
    if (mgr->QuerySwitchStatus(SHORTWORD_KEY)) {
        adapter_->SetParameter("WakeupMode=1");
        ProcDspModel(static_cast<OHOS::HDI::IntelligentVoice::Engine::V1_0::ContentType>(
            OHOS::HDI::IntelligentVoice::Engine::V1_2::SHORT_WORD_DSP_MODEL));
    } else {
        adapter_->SetParameter("WakeupMode=0");
        ProcDspModel(OHOS::HDI::IntelligentVoice::Engine::V1_0::DSP_MODLE);
    }
}

std::string WakeupEngineImpl::GetWakeupPhrase()
{
    std::string wakeupPhrase = HistoryInfoMgr::GetInstance().GetWakeupPhrase();
    if (wakeupPhrase.empty()) {
        INTELL_VOICE_LOG_WARN("no phrase, use default phrase");
        return std::string(DEFAULT_WAKEUP_PHRASE);
    }

    return wakeupPhrase;
}
}
}
