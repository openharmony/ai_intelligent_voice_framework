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
#include "engine_host_manager.h"

#include "iproxy_broker.h"
#include "intell_voice_log.h"
#include "data_operation_callback.h"
#include "intell_voice_util.h"

#define LOG_TAG "EngineHostManager"

using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {
EngineHostManager::~EngineHostManager()
{
    engineHostProxy1_0_ = nullptr;
    engineHostProxy1_1_ = nullptr;
    engineHdiDeathRecipient_ = nullptr;
    dataOprCb_ = nullptr;
}

bool EngineHostManager::Init()
{
    INTELL_VOICE_LOG_INFO("enter");
    engineHostProxy1_0_ = OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineManager::Get();
    if (engineHostProxy1_0_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("engineHostProxy1_0_ is nullptr");
        return false;
    }

    uint32_t majorVer = 0;
    uint32_t minorVer = 0;
    engineHostProxy1_0_->GetVersion(majorVer, minorVer);
    INTELL_VOICE_LOG_INFO("major ver is %{public}u, minor ver is %{public}u", majorVer, minorVer);
    if (CheckHdiVersion(1, 1)(majorVer, minorVer) == VersionCheckType::EQUAL) {
        INTELL_VOICE_LOG_INFO("version is 1.1");
        auto castResult_V1_1 =
            OHOS::HDI::IntelligentVoice::Engine::V1_1::IIntellVoiceEngineManager::CastFrom(engineHostProxy1_0_);
        if (castResult_V1_1 != nullptr) {
            engineHostProxy1_1_ = castResult_V1_1;
        }
    }

    return true;
}

bool EngineHostManager::RegisterEngineHDIDeathRecipient()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (engineHostProxy1_0_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to get engine manager");
        return false;
    }
    sptr<IRemoteObject> object = OHOS::HDI::hdi_objcast<
        OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineManager>(engineHostProxy1_0_);
    if (object == nullptr) {
        INTELL_VOICE_LOG_ERROR("object is nullptr");
        return false;
    }

    engineHdiDeathRecipient_ = new (std::nothrow) IntellVoiceDeathRecipient(
        std::bind(&EngineHostManager::OnEngineHDIDiedCallback, this));
    if (engineHdiDeathRecipient_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("create death recipient failed");
        return false;
    }

    return object->AddDeathRecipient(engineHdiDeathRecipient_);
}

void EngineHostManager::DeregisterEngineHDIDeathRecipient()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (engineHdiDeathRecipient_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("recipient is nullptr");
        return;
    }

    if (engineHostProxy1_0_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("engineHostProxy1_0_ is nullptr");
        return;
    }

    sptr<IRemoteObject> object = OHOS::HDI::hdi_objcast<
        OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineManager>(engineHostProxy1_0_);
    if (object == nullptr) {
        INTELL_VOICE_LOG_ERROR("object is nullptr");
        return;
    }

    object->RemoveDeathRecipient(engineHdiDeathRecipient_);
}

void EngineHostManager::SetDataOprCallback()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (dataOprCb_ != nullptr) {
        INTELL_VOICE_LOG_INFO("already set data opr callback");
        return;
    }

    if (engineHostProxy1_1_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("engineHostProxy1_1_ is nullptr");
        return;
    }

    dataOprCb_ = sptr<OHOS::HDI::IntelligentVoice::Engine::V1_1::IIntellVoiceDataOprCallback>(
        new (std::nothrow) DataOperationCallback());
    if (dataOprCb_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("create data opr callback failed");
        return;
    }

    engineHostProxy1_1_->SetDataOprCallback(dataOprCb_);
}

sptr<IIntellVoiceEngineAdapter> EngineHostManager::CreateEngineAdapter(const IntellVoiceEngineAdapterDescriptor &desc)
{
    if (engineHostProxy1_0_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("engineHostProxy1_0_ is nullptr");
        return nullptr;
    }

    sptr<IIntellVoiceEngineAdapter> adapter = nullptr;
    engineHostProxy1_0_->CreateAdapter(desc, adapter);
    return adapter;
}

void EngineHostManager::ReleaseEngineAdapter(const IntellVoiceEngineAdapterDescriptor &desc)
{
    if (engineHostProxy1_0_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("engineHostProxy1_0_ is nullptr");
        return;
    }
    engineHostProxy1_0_->ReleaseAdapter(desc);
}

void EngineHostManager::OnEngineHDIDiedCallback()
{
    INTELL_VOICE_LOG_INFO("receive engine hdi death recipient");
    _Exit(0);
}
}
}