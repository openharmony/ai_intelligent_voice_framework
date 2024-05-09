/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "enroll_engine.h"
#include <fstream>
#include "securec.h"
#include "intell_voice_log.h"

#include "enroll_adapter_listener.h"
#include "time_util.h"
#include "scope_guard.h"
#include "adapter_callback_service.h"
#include "intell_voice_service_manager.h"
#include "update_engine_utils.h"
#include "engine_host_manager.h"

#define LOG_TAG "EnrollEngine"

using namespace OHOS::IntellVoiceTrigger;
using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::AudioStandard;

namespace OHOS {
namespace IntellVoiceEngine {
static constexpr uint32_t MIN_BUFFER_SIZE = 1280; // 16 * 2 * 40ms
static constexpr uint32_t INTERVAL = 125; // 125 * 40ms = 5s
EnrollEngine::EnrollEngine()
{
    INTELL_VOICE_LOG_INFO("enter");

    capturerOptions_.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_16000;
    capturerOptions_.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions_.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions_.streamInfo.channels = AudioChannel::MONO;
    capturerOptions_.capturerInfo.sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    capturerOptions_.capturerInfo.capturerFlags = 0;
}

EnrollEngine::~EnrollEngine()
{
    INTELL_VOICE_LOG_INFO("enter");
    callback_ = nullptr;
}

void EnrollEngine::OnEnrollEvent(int32_t msgId, int32_t result)
{
    if (msgId == INTELL_VOICE_ENGINE_MSG_ENROLL_COMPLETE) {
        std::thread(&EnrollEngine::OnEnrollComplete, this).detach();
    } else if (msgId == INTELL_VOICE_ENGINE_MSG_COMMIT_ENROLL_COMPLETE) {
        std::lock_guard<std::mutex> lock(mutex_);
        enrollResult_ = result;
        IntellVoiceServiceManager::SetEnrollResult(INTELL_VOICE_ENROLL, result == 0 ? true : false);
    }
}

void EnrollEngine::OnEnrollComplete()
{
    std::lock_guard<std::mutex> lock(mutex_);
    StopAudioSource();
}

bool EnrollEngine::Init(const std::string &param)
{
    if (!EngineUtil::CreateAdapterInner(EngineHostManager::GetInstance(), ENROLL_ADAPTER_TYPE)) {
        INTELL_VOICE_LOG_ERROR("failed to create adapter");
        return false;
    }
    return true;
}

void EnrollEngine::SetCallback(sptr<IRemoteObject> object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    INTELL_VOICE_LOG_INFO("enter");
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return;
    }

    sptr<IIntelligentVoiceEngineCallback> callback = iface_cast<IIntelligentVoiceEngineCallback>(object);
    if (callback == nullptr) {
        INTELL_VOICE_LOG_ERROR("callback is nullptr");
        return;
    }

    std::shared_ptr<IntellVoiceAdapterListener> listener = std::make_shared<EnrollAdapterListener>(callback,
        std::bind(&EnrollEngine::OnEnrollEvent, this, std::placeholders::_1, std::placeholders::_2));
    if (listener == nullptr) {
        INTELL_VOICE_LOG_ERROR("listener is nullptr");
        return;
    }

    callback_ = sptr<IIntellVoiceEngineCallback>(new (std::nothrow) AdapterCallbackService(listener));
    if (callback_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("callback_ is nullptr");
        return;
    }

    adapter_->SetCallback(callback_);
}

int32_t EnrollEngine::Attach(const IntellVoiceEngineInfo &info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    INTELL_VOICE_LOG_INFO("attach");
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }

    SetDspFeatures();
    isPcmFromExternal_ = info.isPcmFromExternal;
    HistoryInfoMgr::GetInstance().SetWakeupPhrase(info.wakeupPhrase);

    IntellVoiceEngineAdapterInfo adapterInfo = {
        .wakeupPhrase = info.wakeupPhrase,
        .minBufSize = info.minBufSize,
        .sampleChannels = info.sampleChannels,
        .bitsPerSample = info.bitsPerSample,
        .sampleRate = info.sampleRate,
    };
    return adapter_->Attach(adapterInfo);
}

int32_t EnrollEngine::Detach(void)
{
    INTELL_VOICE_LOG_INFO("enter");
    std::lock_guard<std::mutex> lock(mutex_);
    StopAudioSource();
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_WARN("already detach");
        return 0;
    }

    if (enrollResult_ == 0) {
        ProcDspModel(OHOS::HDI::IntelligentVoice::Engine::V1_0::DSP_MODLE);
        /* save new version number */
        UpdateEngineUtils::SaveWakeupVesion();
        INTELL_VOICE_LOG_INFO("enroll save version");
    }

    int32_t ret = adapter_->Detach();
    ReleaseAdapterInner(EngineHostManager::GetInstance());
    return ret;
}

int32_t EnrollEngine::Start(bool isLast)
{
    std::lock_guard<std::mutex> lock(mutex_);
    INTELL_VOICE_LOG_INFO("enter");
    if (audioSource_ != nullptr) {
        INTELL_VOICE_LOG_ERROR("audioSource_ existed, wait for last start to finish");
        return -1;
    }

    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }

    StartInfo info = {
        .isLast = isLast,
    };
    if (adapter_->Start(info)) {
        INTELL_VOICE_LOG_ERROR("start adapter failed");
        return -1;
    }

    if (isPcmFromExternal_) {
        INTELL_VOICE_LOG_INFO("pcm is from external");
        return 0;
    }

    if (!StartAudioSource()) {
        INTELL_VOICE_LOG_ERROR("start audio source failed");
        adapter_->Stop();
        return -1;
    }

    INTELL_VOICE_LOG_INFO("exit");
    return 0;
}

int32_t EnrollEngine::Stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    StopAudioSource();

    return EngineUtil::Stop();
}

int32_t EnrollEngine::SetParameter(const std::string &keyValueList)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (SetParameterInner(keyValueList)) {
        INTELL_VOICE_LOG_INFO("inner parameter");
        return 0;
    }

    INTELL_VOICE_LOG_INFO("EnrollEngine SetParameter:%{public}s", keyValueList.c_str());

    return EngineUtil::SetParameter(keyValueList);
}

bool EnrollEngine::SetParameterInner(const std::string &keyValueList)
{
    HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();

    std::map<std::string, std::string> kvpairs;
    EngineUtil::SplitStringToKVPair(keyValueList, kvpairs);
    for (auto it : kvpairs) {
        if (it.first == std::string("wakeup_bundle_name")) {
            INTELL_VOICE_LOG_INFO("set wakeup bundle name:%{public}s", it.second.c_str());
            historyInfoMgr.SetWakeupEngineBundleName(it.second);
            return true;
        }
        if (it.first == std::string("wakeup_ability_name")) {
            INTELL_VOICE_LOG_INFO("set wakeup ability name:%{public}s", it.second.c_str());
            historyInfoMgr.SetWakeupEngineAbilityName(it.second);
            return true;
        }
        if (it.first == std::string("language")) {
            INTELL_VOICE_LOG_INFO("set language:%{public}s", it.second.c_str());
            historyInfoMgr.SetLanguage(it.second);
            continue;
        }
        if (it.first == std::string("area")) {
            INTELL_VOICE_LOG_INFO("set area:%{public}s", it.second.c_str());
            historyInfoMgr.SetArea(it.second);
            continue;
        }
    }

    return false;
}

std::string EnrollEngine::GetParameter(const std::string &key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return EngineUtil::GetParameter(key);
}

int32_t EnrollEngine::WriteAudio(const uint8_t *buffer, uint32_t size)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return EngineUtil::WriteAudio(buffer, size);
}

int32_t EnrollEngine::Evaluate(const std::string &word, EvaluationResultInfo &info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return EngineUtil::Evaluate(word, info);
}

bool EnrollEngine::StartAudioSource()
{
    auto listener = std::make_unique<AudioSourceListener>([&] (uint8_t *buffer, uint32_t size, bool isEnd) {
        if ((adapter_ != nullptr) && (!isEnd)) {
            std::vector<uint8_t> audioBuff(&buffer[0], &buffer[size]);
            adapter_->WriteAudio(audioBuff);
        }}, [&] () {
            INTELL_VOICE_LOG_INFO("end of pcm");
            if (adapter_ != nullptr) {
                adapter_->SetParameter("end_of_pcm=true");
            }
        });
    if (listener == nullptr) {
        INTELL_VOICE_LOG_ERROR("create listener failed");
        return false;
    }

    audioSource_ = std::make_unique<AudioSource>(MIN_BUFFER_SIZE, INTERVAL, std::move(listener),
        capturerOptions_);
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

void EnrollEngine::StopAudioSource()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (audioSource_ != nullptr) {
        INTELL_VOICE_LOG_INFO("stop audio sopurce");
        audioSource_->Stop();
        audioSource_ = nullptr;
    }
}
}
}
