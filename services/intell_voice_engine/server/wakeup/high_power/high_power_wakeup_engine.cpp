/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "high_power_wakeup_engine.h"
#include "idevmgr_hdi.h"
#include "ipc_skeleton.h"
#include "v1_2/intell_voice_engine_types.h"
#include "intell_voice_engine_manager.h"
#include "intell_voice_log.h"
#include "engine_host_manager.h"
#include "adapter_callback_service.h"

#define LOG_TAG "HighPowerWakeupEngine"

using namespace OHOS::IntellVoiceUtils;
using OHOS::HDI::DeviceManager::V1_0::IDeviceManager;
using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;

namespace OHOS {
namespace IntellVoiceEngine {
static const std::string CALLBACK_EXIST = "is_callback_exist";
static const std::string HIGH_POWER_THREAD_NAME = "HighPowerEngThread";
static constexpr uint32_t MAX_WAKEUP_TASK_NUM = 200;

HighPowerWakeupEngine::HighPowerWakeupEngine()
{
    INTELL_VOICE_LOG_INFO("enter");
    adapterListener_ = std::make_shared<HighPowerAdapterListener>();
    if (adapterListener_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapterListener_ is nullptr");
    }
}

HighPowerWakeupEngine::~HighPowerWakeupEngine()
{
    INTELL_VOICE_LOG_INFO("enter");
}

bool HighPowerWakeupEngine::Init(const std::string & /* param */, bool reEnroll)
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
    return true;
}

void HighPowerWakeupEngine::SetCallback(sptr<IRemoteObject> object)
{
    std::unique_lock<std::mutex> lock(mutex_);
    sptr<IIntelligentVoiceEngineCallback> callback = iface_cast<IIntelligentVoiceEngineCallback>(object);
    if (callback == nullptr) {
        INTELL_VOICE_LOG_WARN("clear callback");
    }
    if (adapterListener_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapterListener_ is nullptr");
        return;
    }
    adapterListener_->SetCallback(callback);
}

std::string HighPowerWakeupEngine::GetParameter(const std::string &key)
{
    if (key == CALLBACK_EXIST) {
        if (adapterListener_ == nullptr) {
            INTELL_VOICE_LOG_ERROR("adapterListener_ is nullptr");
            return "false";
        } else {
            return adapterListener_->GetCallbackStatus();
        }
    }
    return "";
}

int32_t HighPowerWakeupEngine::Detach(void)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (adapter_ != nullptr) {
        adapter_->Detach();
        ReleaseAdapterInner(EngineHostManager::GetInstance());
    }
    return 0;
}

bool HighPowerWakeupEngine::SetCallbackInner()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return false;
    }

    if (adapterListener_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter listener is nullptr");
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
}  // namespace IntellVoice
}  // namespace OHOS
