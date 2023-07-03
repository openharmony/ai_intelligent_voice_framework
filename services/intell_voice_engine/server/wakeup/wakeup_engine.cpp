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
#include "wakeup_engine.h"
#include <fstream>
#include "securec.h"
#include "intell_voice_log.h"

#include "v1_0/iintell_voice_engine_manager.h"
#include "v1_0/iintell_voice_engine_callback.h"

#include "time_util.h"
#include "scope_guard.h"
#include "adapter_callback_service.h"
#include "intell_voice_service_manager.h"
#include "ability_manager_client.h"
#include "memory_guard.h"

using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::AudioStandard;
#define LOG_TAG "WakeupEngine"

namespace OHOS {
namespace IntellVoiceEngine {
static constexpr uint32_t MIN_BUFFER_SIZE = 1280;
static constexpr uint32_t INTERVAL = 50;
static const std::string RECOGNITION_FILE = "/data/data/recognition.pcm";

WakeupEngine::WakeupEngine()
{
    INTELL_VOICE_LOG_INFO("enter");

    capturerOptions_.streamInfo.channels = AudioChannel::MONO;
    capturerOptions_.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_16000;
    capturerOptions_.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions_.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions_.capturerInfo.capturerFlags = 0;
}

WakeupEngine::~WakeupEngine()
{
    auto mgr = IIntellVoiceEngineManager::Get();
    if (mgr != nullptr) {
        mgr->ReleaseAdapter(desc_);
    }
    adapter_ = nullptr;
    callback_ = nullptr;
}

void WakeupEngine::OnDetected()
{
    INTELL_VOICE_LOG_INFO("on detected");
    StartAbility();
    SetParameter("VprTrdType=0;WakeupScene=0");
    Start(true);
}

void WakeupEngine::OnWakeupEvent(int32_t msgId, int32_t result)
{
    if (msgId == INTELL_VOICE_ENGINE_MSG_RECOGNIZE_COMPLETE) {
        std::thread(&WakeupEngine::OnWakeupRecognition, this).detach();
    }
}

void WakeupEngine::OnWakeupRecognition()
{
    INTELL_VOICE_LOG_INFO("on wakeup recognition");
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (fileSource_ != nullptr) {
            fileSource_->Stop();
            fileSource_ = nullptr;
        }
    }

    const auto &manager = IntellVoiceServiceManager::GetInstance();
    if (manager != nullptr) {
        manager->StartDetection();
    }

    Stop();
}

bool WakeupEngine::SetCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return false;
    }

    adapterListener_ = std::make_shared<WakeupAdapterListener>(
        std::bind(&WakeupEngine::OnWakeupEvent, this, std::placeholders::_1, std::placeholders::_2));
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

bool WakeupEngine::Init()
{
    desc_.adapterType = WAKEUP_ADAPTER_TYPE;
    auto mgr = IIntellVoiceEngineManager::Get();
    if (mgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to get engine manager");
        return false;
    }

    mgr->CreateAdapter(desc_, adapter_);
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return false;
    }

    if (!SetCallback()) {
        INTELL_VOICE_LOG_ERROR("failed to set callback");
        return false;
    }

    IntellVoiceEngineInfo info = {
        .wakeupPhrase = "\xE5\xB0\x8F\xE8\x89\xBA\xE5\xB0\x8F\xE8\x89\xBA",
        .isPcmFromExternal = false,
        .minBufSize = 1280,
        .sampleChannels = 1,
        .bitsPerSample = 16,
        .sampleRate = 16000,
    };

    if (Attach(info) != 0) {
        INTELL_VOICE_LOG_ERROR("failed to attach");
        return false;
    }

    return true;
}

void WakeupEngine::SetCallback(sptr<IRemoteObject> object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (adapterListener_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter listener is nullptr");
        return;
    }

    sptr<IIntelligentVoiceEngineCallback> callback = iface_cast<IIntelligentVoiceEngineCallback>(object);
    if (callback == nullptr) {
        INTELL_VOICE_LOG_ERROR("callback is nullptr");
        return;
    }

    adapterListener_->SetCallback(callback);
}

int32_t WakeupEngine::Attach(const IntellVoiceEngineInfo &info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    INTELL_VOICE_LOG_INFO("attach");
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

int32_t WakeupEngine::Detach(void)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }
    return adapter_->Detach();
}

int32_t WakeupEngine::Start(bool isLast)
{
    std::lock_guard<std::mutex> lock(mutex_);
    INTELL_VOICE_LOG_INFO("enter");
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }

    if (IntellVoiceServiceManager::GetInstance()->ApplyArbitration(INTELL_VOICE_WAKEUP, ENGINE_EVENT_START) !=
        ARBITRATION_OK) {
        INTELL_VOICE_LOG_ERROR("policy manager reject to start engine");
        return 0;
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

    if (!StartFileSource()) {
        INTELL_VOICE_LOG_ERROR("start file source failed");
        adapter_->Stop();
        return -1;
    }

    INTELL_VOICE_LOG_INFO("exit");
    return 0;
}

void WakeupEngine::StartAbility()
{
    AAFwk::Want want;
    const std::unique_ptr<HistoryInfoMgr> &historyInfoMgr =
        IntellVoiceServiceManager::GetInstance()->GetHistoryInfoMgr();
    if (historyInfoMgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("historyInfoMgr is nullptr");
        return;
    }

    std::string bundleName = historyInfoMgr->GetWakeupEngineBundleName();
    std::string abilityName = historyInfoMgr->GetWakeupEngineAbilityName();
    INTELL_VOICE_LOG_INFO("bundleName:%{public}s, abilityName:%{public}s", bundleName.c_str(), abilityName.c_str());
    want.SetElementName(bundleName, abilityName);
    want.SetParam("serviceName", std::string("intell_voice"));
    AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want);
}

int32_t WakeupEngine::SetParameter(const std::string &keyValueList)
{
    if (SetParameterInner(keyValueList)) {
        INTELL_VOICE_LOG_INFO("inner parameter");
        return 0;
    }

    return EngineBase::SetParameter(keyValueList);
}

bool WakeupEngine::SetParameterInner(const std::string &keyValueList)
{
    std::lock_guard<std::mutex> lock(mutex_);

    const auto &manager = IntellVoiceServiceManager::GetInstance();
    if (manager == nullptr) {
        INTELL_VOICE_LOG_ERROR();
        return false;
    }

    std::map<std::string, std::string> kvpairs;
    SplitStringToKVPair(keyValueList, kvpairs);
    for (auto it : kvpairs) {
        if (it.first == std::string("start_stream")) {
            INTELL_VOICE_LOG_INFO("start stream:%{public}s", it.second.c_str());
            manager->StopDetection();
            return true;
        }
        if (it.first == std::string("stop_stream")) {
            INTELL_VOICE_LOG_INFO("stop stream:%{public}s", it.second.c_str());
            manager->StartDetection();
            return true;
        }
    }

    return false;
}

bool WakeupEngine::StartFileSource()
{
    auto listener = std::make_unique<FileSourceListener>(
        [&](uint8_t *buffer, uint32_t size) {
            if (adapter_ != nullptr) {
                std::vector<uint8_t> audioBuff(&buffer[0], &buffer[size]);
                adapter_->WriteAudio(audioBuff);
            }
        },
        [&](bool isError) {
            INTELL_VOICE_LOG_INFO("end of pcm, isError:%d", isError);
            if (adapter_ != nullptr) {
                adapter_->SetParameter("end_of_pcm=true");
            }
        });
    if (listener == nullptr) {
        INTELL_VOICE_LOG_ERROR("create listener failed");
        return false;
    }

    fileSource_ = std::make_unique<FileSource>(MIN_BUFFER_SIZE, INTERVAL, RECOGNITION_FILE, std::move(listener));
    if (fileSource_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("create file source failed");
        return false;
    }

    if (!fileSource_->Start()) {
        INTELL_VOICE_LOG_ERROR("start capturer failed");
        fileSource_ = nullptr;
        return false;
    }

    return true;
}
}  // namespace IntellVoice
}  // namespace OHOS
