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
#include "headset_wakeup_engine_impl.h"
#include "audio_system_manager.h"
#include "adapter_callback_service.h"
#include "intell_voice_log.h"
#include "history_info_mgr.h"
#include "intell_voice_util.h"
#include "intell_voice_service_manager.h"
#include "headset_host_manager.h"

#define LOG_TAG "HeadsetWakeupEngineImpl"

using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::AudioStandard;
using namespace std;

namespace OHOS {
namespace IntellVoiceEngine {
static constexpr int64_t RECOGNIZING_TIMEOUT_US = 10 * 1000 * 1000; //10s
static constexpr int64_t RECOGNIZE_COMPLETE_TIMEOUT_US = 1 * 1000; //1ms
static constexpr int64_t READ_CAPTURER_TIMEOUT_US = 10 * 1000 * 1000; //10s

HeadsetWakeupEngineImpl::HeadsetWakeupEngineImpl() : ModuleStates(State(IDLE), "HeadsetWakeupEngineImpl")
{
}

HeadsetWakeupEngineImpl::~HeadsetWakeupEngineImpl()
{
}

bool HeadsetWakeupEngineImpl::Init()
{
    if (!InitStates()) {
        INTELL_VOICE_LOG_ERROR("init state failed");
        return false;
    }

    adapterListener_ = std::make_shared<WakeupAdapterListener>(
        std::bind(&HeadsetWakeupEngineImpl::OnWakeupEvent, this, std::placeholders::_1, std::placeholders::_2));
    if (adapterListener_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapterListener_ is nullptr");
        return false;
    }

    return true;
}

bool HeadsetWakeupEngineImpl::InitStates()
{
    for (int i = IDLE; i <= READ_CAPTURER; i++) {
        ForState(State(i))
            .ACT(SET_LISTENER, HandleSetListener)
            .ACT(SET_PARAM, HandleSetParam);
    }

    ForState(IDLE)
        .ACT(INIT, HandleInit)
        .ACT(RESET_ADAPTER, HandleResetAdapter);

    ForState(INITIALIZING)
        .ACT(INIT_DONE, HandleInitDone);

    ForState(INITIALIZED)
        .ACT(START_RECOGNIZE, HandleStart);

    ForState(RECOGNIZING)
        .WaitUntil(RECOGNIZING_TIMEOUT, std::bind(&HeadsetWakeupEngineImpl::HandleRecognizingTimeout,
            this, std::placeholders::_1, std::placeholders::_2), RECOGNIZING_TIMEOUT_US)
        .ACT(STOP_RECOGNIZE, HandleStop)
        .ACT(RECOGNIZE_COMPLETE, HandleRecognizeComplete);

    ForState(RECOGNIZED)
        .WaitUntil(RECOGNIZE_COMPLETE_TIMEOUT, std::bind(&HeadsetWakeupEngineImpl::HandleStopCapturer,
            this, std::placeholders::_1, std::placeholders::_2), RECOGNIZE_COMPLETE_TIMEOUT_US)
        .ACT(START_CAPTURER, HandleStartCapturer);

    ForState(READ_CAPTURER)
        .WaitUntil(READ_CAPTURER_TIMEOUT, std::bind(&HeadsetWakeupEngineImpl::HandleStopCapturer,
            this, std::placeholders::_1, std::placeholders::_2), READ_CAPTURER_TIMEOUT_US)
        .ACT(READ, HandleRead)
        .ACT(STOP_CAPTURER, HandleStopCapturer);

    FromState(INITIALIZING, READ_CAPTURER)
        .ACT(RELEASE_ADAPTER, HandleRelease)
        .ACT(RELEASE, HandleRelease);

    return IsStatesInitSucc();
}

int32_t HeadsetWakeupEngineImpl::Handle(const StateMsg &msg)
{
    if (!IsStatesInitSucc()) {
        INTELL_VOICE_LOG_ERROR("failed to init state");
        return -1;
    }

    return ModuleStates::HandleMsg(msg);
}

bool HeadsetWakeupEngineImpl::SetCallbackInner()
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

int32_t HeadsetWakeupEngineImpl::AttachInner(const IntellVoiceEngineInfo &info)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }

    IntellVoiceEngineAdapterInfo adapterInfo = {
    };

    return adapter_->Attach(adapterInfo);
}

void HeadsetWakeupEngineImpl::OnWakeupEvent(int32_t msgId, int32_t result)
{
    INTELL_VOICE_LOG_INFO("enter, msgId:%{public}d, result:%{public}d", msgId, result);
    if (msgId == INTELL_VOICE_ENGINE_MSG_INIT_DONE) {
        std::thread(&HeadsetWakeupEngineImpl::OnInitDone, this, result).detach();
    } else if (msgId == HDI::IntelligentVoice::Engine::V1_2::INTELL_VOICE_ENGINE_MSG_HEADSET_RECOGNIZE_COMPLETE) {
        std::thread(&HeadsetWakeupEngineImpl::OnWakeupRecognition, this, result).detach();
    } else {
    }
}

void HeadsetWakeupEngineImpl::OnInitDone(int32_t result)
{
    INTELL_VOICE_LOG_INFO("enter, result:%{public}d", result);
    StateMsg msg(INIT_DONE, &result, sizeof(int32_t));
    Handle(msg);
}

void HeadsetWakeupEngineImpl::OnWakeupRecognition(int32_t result)
{
    INTELL_VOICE_LOG_INFO("enter, result:%{public}d", result);
    StateMsg msg(RECOGNIZE_COMPLETE, &result, sizeof(int32_t));
    Handle(msg);
}

int32_t HeadsetWakeupEngineImpl::HandleInit(const StateMsg & /* msg */, State &nextState)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (!EngineUtil::CreateAdapterInner(HeadsetHostManager::GetInstance(), WAKEUP_ADAPTER_TYPE)) {
        INTELL_VOICE_LOG_ERROR("failed to create adapter");
        return -1;
    }

    if (!SetCallbackInner()) {
        INTELL_VOICE_LOG_ERROR("failed to set callback");
        return -1;
    }

    EngineUtil::SetLanguage();
    EngineUtil::SetArea();
    adapter_->SetParameter("model_path=/data/local/tmp/encoder.om");
    IntellVoiceEngineInfo info = {
    };

    if (AttachInner(info) != 0) {
        INTELL_VOICE_LOG_ERROR("failed to attach");
        return -1;
    }

    nextState = State(INITIALIZING);
    return 0;
}

int32_t HeadsetWakeupEngineImpl::HandleInitDone(const StateMsg &msg, State &nextState)
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

int32_t HeadsetWakeupEngineImpl::HandleSetListener(const StateMsg &msg, State & /* nextState */)
{
    SetListenerMsg *listenerMsg = reinterpret_cast<SetListenerMsg *>(msg.inMsg);
    if (listenerMsg == nullptr || adapterListener_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("listenerMsg or adapter listener is nullptr");
        return -1;
    }

    adapterListener_->SetCallback(listenerMsg->callback);
    return 0;
}

int32_t HeadsetWakeupEngineImpl::HandleStart(const StateMsg & /* msg */, State &nextState)
{
    INTELL_VOICE_LOG_INFO("enter");
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

    if (!StartAudioSource()) {
        INTELL_VOICE_LOG_ERROR("start audio source failed");
        adapter_->Stop();
        return -1;
    }

    INTELL_VOICE_LOG_INFO("exit");
    nextState = State(RECOGNIZING);
    return 0;
}

void HeadsetWakeupEngineImpl::ReadThread()
{
    while (isReading_.load()) {
        std::vector<uint8_t> audioStream;
        int ret = ReadHeadsetStream(audioStream);
        if (ret == -1) {
            INTELL_VOICE_LOG_INFO("finish reading");
            adapter_->SetParameter("end_of_pcm=true");
            break;
        }
        adapter_->WriteAudio(audioStream);
        WakeupSourceProcess::Write({ audioStream });
    }
}

bool HeadsetWakeupEngineImpl::StartAudioSource()
{
    INTELL_VOICE_LOG_INFO("enter");
    isReading_.store(true);

    WakeupSourceProcess::Init(1);

    std::thread t1(std::bind(&HeadsetWakeupEngineImpl::ReadThread, this));
    readThread_ = std::move(t1);
    return true;
}

void HeadsetWakeupEngineImpl::StopAudioSource()
{
    if (!isReading_.load()) {
        INTELL_VOICE_LOG_INFO("already stop");
        return;
    }
    isReading_.store(false);
    readThread_.join();
    StopReadingStream();
    WakeupSourceProcess::Release();
}

int32_t HeadsetWakeupEngineImpl::HandleStop(const StateMsg & /* msg */, State &nextState)
{
    StopAudioSource();
    EngineUtil::Stop();
    nextState = State(INITIALIZED);
    return 0;
}

int32_t HeadsetWakeupEngineImpl::HandleRecognizeComplete(const StateMsg &msg, State &nextState)
{
    EngineUtil::Stop();
    int32_t *result = reinterpret_cast<int32_t *>(msg.inMsg);
    if (result == nullptr) {
        INTELL_VOICE_LOG_ERROR("result is nullptr");
        return -1;
    }
    if (*result != 0) {
        INTELL_VOICE_LOG_INFO("wakeup failed");
        StopAudioSource();
        NotifyVerifyResult(false);
        nextState = State(INITIALIZED);
    } else {
        NotifyVerifyResult(true);
        nextState = State(RECOGNIZED);
    }
    return 0;
}

int32_t HeadsetWakeupEngineImpl::HandleStartCapturer(const StateMsg &msg, State &nextState)
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

int32_t HeadsetWakeupEngineImpl::HandleRead(const StateMsg &msg, State & /* nextState */)
{
    CapturerData *capturerData = reinterpret_cast<CapturerData *>(msg.outMsg);
    auto ret = WakeupSourceProcess::Read(capturerData->data, channels_);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("read capturer data failed");
        return ret;
    }

    ResetTimerDelay();
    return 0;
}

int32_t HeadsetWakeupEngineImpl::HandleStopCapturer(const StateMsg & /* msg */, State &nextState)
{
    INTELL_VOICE_LOG_INFO("enter");
    StopAudioSource();
    nextState = State(INITIALIZED);
    return 0;
}

int32_t HeadsetWakeupEngineImpl::HandleRecognizingTimeout(const StateMsg & /* msg */, State &nextState)
{
    INTELL_VOICE_LOG_INFO("enter");
    StopAudioSource();
    EngineUtil::Stop();
    nextState = State(INITIALIZED);
    return 0;
}

int32_t HeadsetWakeupEngineImpl::HandleResetAdapter(const StateMsg & /* msg */, State &nextState)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (!EngineUtil::CreateAdapterInner(HeadsetHostManager::GetInstance(), WAKEUP_ADAPTER_TYPE)) {
        INTELL_VOICE_LOG_ERROR("failed to create adapter");
        return -1;
    }

    adapter_->SetCallback(callback_);
    EngineUtil::SetLanguage();
    EngineUtil::SetArea();

    IntellVoiceEngineAdapterInfo adapterInfo = {
    };

    if (adapter_->Attach(adapterInfo) != 0) {
        INTELL_VOICE_LOG_ERROR("failed to attach");
        EngineUtil::ReleaseAdapterInner(HeadsetHostManager::GetInstance());
        return -1;
    }

    nextState = State(INITIALIZING);
    return 0;
}

int32_t HeadsetWakeupEngineImpl::HandleRelease(const StateMsg & /* msg */, State &nextState)
{
    StopAudioSource();
    if (adapter_ != nullptr) {
        adapter_->Detach();
        ReleaseAdapterInner(HeadsetHostManager::GetInstance());
    }
    nextState = State(IDLE);
    return 0;
}

int32_t HeadsetWakeupEngineImpl::HandleSetParam(const StateMsg &msg, State & /* nextState */)
{
    StringParam *param = reinterpret_cast<StringParam *>(msg.inMsg);
    if (param == nullptr) {
        INTELL_VOICE_LOG_INFO("param is nullptr");
        return -1;
    }

    return EngineUtil::SetParameter(param->strParam);
}
}
}