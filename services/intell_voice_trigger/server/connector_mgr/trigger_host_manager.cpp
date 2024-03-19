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
#include "trigger_host_manager.h"

#include "iproxy_broker.h"
#include "intell_voice_log.h"
#include "intell_voice_util.h"
#include "intell_voice_death_recipient.h"

#define LOG_TAG "TriggerHostManager"

using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceTrigger {
TriggerHostManager::TriggerHostManager()
{
}

TriggerHostManager::~TriggerHostManager()
{
    adapter_ = nullptr;
    adapterV1_1_ = nullptr;
}

bool TriggerHostManager::Init()
{
    if (triggerHostProxy_ != nullptr) {
        INTELL_VOICE_LOG_INFO("already init");
        return true;
    }

    triggerHostProxy_ = OHOS::HDI::IntelligentVoice::Trigger::V1_0::IIntellVoiceTriggerManager::Get();
    if (triggerHostProxy_ == nullptr) {
        INTELL_VOICE_LOG_WARN("can not get intell voice trigger manager");
        return false;
    }

    uint32_t majorVer = 0;
    uint32_t minorVer = 0;
    triggerHostProxy_->GetVersion(majorVer, minorVer);

    if (IntellVoiceUtil::GetHdiVersionId(majorVer, minorVer) == IntellVoiceUtil::GetHdiVersionId(1, 1)) {
        INTELL_VOICE_LOG_INFO("version is 1.1");
        auto castResult_V1_1 =
            OHOS::HDI::IntelligentVoice::Trigger::V1_1::IIntellVoiceTriggerManager::CastFrom(triggerHostProxy_);
        if (castResult_V1_1 != nullptr) {
            triggerHostProxyV1_1_ = castResult_V1_1;
        }
    }

    return true;
}

bool TriggerHostManager::RegisterTriggerHDIDeathRecipient()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (triggerHostProxy_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger host proxy is nullptr");
        return false;
    }

    sptr<IRemoteObject> object = OHOS::HDI::hdi_objcast<
        OHOS::HDI::IntelligentVoice::Trigger::V1_0::IIntellVoiceTriggerManager>(triggerHostProxy_);
    if (object == nullptr) {
        INTELL_VOICE_LOG_ERROR("object is nullptr");
        return false;
    }

    sptr<IntellVoiceDeathRecipient> recipient = new (std::nothrow) IntellVoiceDeathRecipient(
        std::bind(&TriggerHostManager::OnTriggerHDIDiedCallback));
    if (recipient == nullptr) {
        INTELL_VOICE_LOG_ERROR("create death recipient failed");
        return false;
    }

    return object->AddDeathRecipient(recipient);
}

bool TriggerHostManager::LoadTriggerAdapter(const IntellVoiceTriggerAdapterDsecriptor &desc)
{
    if (triggerHostProxy_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger proxy is nullptr");
        return false;
    }

    if (triggerHostProxyV1_1_ != nullptr) {
        triggerHostProxyV1_1_->LoadAdapter_V1_1(desc, adapterV1_1_);
        if (adapterV1_1_ == nullptr) {
            INTELL_VOICE_LOG_ERROR("load adapterV1_1_ failed");
            return false;
        }
        adapter_ = adapterV1_1_;
        return true;
    }

    triggerHostProxy_->LoadAdapter(desc, adapter_);
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("load adapter_ failed");
        return false;
    }
    return true;
}

void TriggerHostManager::UnloadTriggerAdapter(const IntellVoiceTriggerAdapterDsecriptor &desc)
{
    if (triggerHostProxy_ != nullptr) {
        triggerHostProxy_->UnloadAdapter(desc);
    }
    adapter_ = nullptr;
    adapterV1_1_ = nullptr;
}

void TriggerHostManager::OnTriggerHDIDiedCallback()
{
    INTELL_VOICE_LOG_INFO("receive trigger hdi death recipient, restart sa");
    _Exit(0);
}
}
}
