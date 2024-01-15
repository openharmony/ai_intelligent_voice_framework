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
#ifndef INTELL_VOICE_TRIGGER_HOST_MANAGER_H
#define INTELL_VOICE_TRIGGER_HOST_MANAGER_H

#include <iservmgr_hdi.h>
#include "v1_0/iintell_voice_trigger_manager.h"
#include "v1_1/iintell_voice_trigger_manager.h"

namespace OHOS {
namespace IntellVoiceTrigger {
using OHOS::HDI::IntelligentVoice::Trigger::V1_0::IntellVoiceTriggerAdapterDsecriptor;

class TriggerHostManager {
public:
    TriggerHostManager();
    ~TriggerHostManager();

protected:
    bool Init();
    bool RegisterTriggerHDIDeathRecipient();
    bool LoadTriggerAdapter(const IntellVoiceTriggerAdapterDsecriptor &desc);
    void UnloadTriggerAdapter(const IntellVoiceTriggerAdapterDsecriptor &desc);
    const sptr<OHOS::HDI::IntelligentVoice::Trigger::V1_0::IIntellVoiceTriggerAdapter> &GetAdapter() const
    {
        return adapter_;
    }
    const sptr<OHOS::HDI::IntelligentVoice::Trigger::V1_1::IIntellVoiceTriggerAdapter> &GetAdapterV1_1() const
    {
        return adapterV1_1_;
    }

private:
    static void OnTriggerHDIDiedCallback();

    sptr<OHOS::HDI::IntelligentVoice::Trigger::V1_0::IIntellVoiceTriggerManager> triggerHostProxy_ = nullptr;
    sptr<OHOS::HDI::IntelligentVoice::Trigger::V1_1::IIntellVoiceTriggerManager> triggerHostProxyV1_1_ = nullptr;

    sptr<OHOS::HDI::IntelligentVoice::Trigger::V1_0::IIntellVoiceTriggerAdapter> adapter_ = nullptr;
    sptr<OHOS::HDI::IntelligentVoice::Trigger::V1_1::IIntellVoiceTriggerAdapter> adapterV1_1_ = nullptr;
};
}  // namespace IntellVoiceTrigger
}  // namespace OHOS
#endif