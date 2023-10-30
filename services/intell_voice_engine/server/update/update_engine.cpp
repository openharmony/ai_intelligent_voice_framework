/*
 * Copyright (c) 2023 Huawei Device Co., Ltd. 2023-2023.
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
#include "update_engine.h"
#include <fstream>
#include "securec.h"
#include "intell_voice_log.h"

#include "v1_0/iintell_voice_engine_manager.h"
#include "v1_0/iintell_voice_engine_callback.h"

#include "update_adapter_listener.h"
#include "time_util.h"
#include "scope_guard.h"
#include "trigger_manager.h"
#include "adapter_callback_service.h"
#include "intell_voice_service_manager.h"

#define LOG_TAG "UpdateEngine"

using namespace OHOS::IntellVoiceTrigger;
using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::AudioStandard;

namespace OHOS {
namespace IntellVoiceEngine {

UpdateEngine::UpdateEngine()
{
    INTELL_VOICE_LOG_INFO("enter");
}

UpdateEngine::~UpdateEngine()
{
    INTELL_VOICE_LOG_INFO("enter");
    auto mgr = IIntellVoiceEngineManager::Get();
    if (mgr != nullptr) {
        mgr->ReleaseAdapter(desc_);
    }
    adapter_ = nullptr;
    callback_ = nullptr;
}

void UpdateEngine::OnEnrollEvent(int32_t msgId, int32_t result)
{
    if (msgId == INTELL_VOICE_ENGINE_MSG_COMMIT_ENROLL_COMPLETE) {
        std::lock_guard<std::mutex> lock(mutex_);
        INTELL_VOICE_LOG_INFO("INTELL_VOICE_ENGINE_MSG_COMMIT_ENROLL_COMPLETE result %{public}d", result);
        enrollResult_ = result;
        std::thread([=]() {
            const auto &manager = IntellVoiceServiceManager::GetInstance();
            if (manager != nullptr) {
                manager->OnUpdateComplete(result);
            }
        }).detach();
    }
}

bool UpdateEngine::Init()
{
    desc_.adapterType = UPDATE_ADAPTER_TYPE;
    sptr<IRemoteObject> object;

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

    SetCallback(object);

    IntellVoiceEngineInfo info = {
        .wakeupPhrase = "\xE5\xB0\x8F\xE8\x89\xBA\xE5\xB0\x8F\xE8\x89\xBA",
        .minBufSize = 1280,
        .sampleChannels = 1,
        .bitsPerSample = 16,
        .sampleRate = 16000,
    };
    int ret = Attach(info);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("attach err");
        return false;
    }

    return true;
}

void UpdateEngine::SetCallback(sptr<IRemoteObject> object)
{
    std::lock_guard<std::mutex> lock(mutex_);

    callback_ = sptr<IIntellVoiceEngineCallback>(new (std::nothrow) UpdateEngineCallback(
        std::bind(&UpdateEngine::OnEnrollEvent, this, std::placeholders::_1, std::placeholders::_2)));
    if (callback_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("callback_ is nullptr");
        return;
    }

    adapter_->SetCallback(callback_);
}

int32_t UpdateEngine::Attach(const IntellVoiceEngineInfo &info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    INTELL_VOICE_LOG_INFO("attach");
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }

    IntellVoiceEngineAdapterInfo adapterInfo = {
        .wakeupPhrase = info.wakeupPhrase,
        .minBufSize = info.minBufSize,
        .sampleChannels = info.sampleChannels,
        .bitsPerSample = info.bitsPerSample,
        .sampleRate = info.sampleRate,
    };
    return adapter_->Attach(adapterInfo);
}

int32_t UpdateEngine::Detach(void)
{
    INTELL_VOICE_LOG_INFO("enter");
    std::lock_guard<std::mutex> lock(mutex_);
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }

    if (enrollResult_ == 0) {
        ProcDspModel();
        /* save new version number */
        if (IntellVoiceServiceManager::GetInstance() != NULL) {
            IntellVoiceServiceManager::GetInstance()->SaveWakeupVesion();
            INTELL_VOICE_LOG_INFO("update save version");
        }
    }

    return adapter_->Detach();
}

int32_t UpdateEngine::Start(bool isLast)
{
    std::lock_guard<std::mutex> lock(mutex_);
    INTELL_VOICE_LOG_INFO("enter");
    return 0;
}

int32_t UpdateEngine::Stop()
{
    INTELL_VOICE_LOG_INFO("enter");
    return EngineBase::Stop();
}

int32_t UpdateEngine::SetParameter(const std::string &keyValueList)
{
    return EngineBase::SetParameter(keyValueList);
}
}
}