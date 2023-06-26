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
#ifndef INTELL_VOICE_TRIGGER_CALLBACK_IMPL_H
#define INTELL_VOICE_TRIGGER_CALLBACK_IMPL_H

#include "v1_0/iintell_voice_trigger_callback.h"
#include "i_intell_voice_trigger_adapter_listener.h"

namespace OHOS {
namespace IntellVoiceTrigger {
using OHOS::HDI::IntelligentVoice::Trigger::V1_0::IntellVoiceRecognitionEvent;
using OHOS::HDI::IntelligentVoice::Trigger::V1_0::IIntellVoiceTriggerCallback;

class TriggerCallbackImpl final : public IIntellVoiceTriggerCallback {
public:
    explicit TriggerCallbackImpl(std::shared_ptr<IIntellVoiceTriggerAdapterListener> listener);
    virtual ~TriggerCallbackImpl() = default;

    int32_t OnRecognitionHdiEvent(const IntellVoiceRecognitionEvent &event, int32_t cookie) override;

private:
    std::shared_ptr<IIntellVoiceTriggerAdapterListener> listener_ = nullptr;
};
}
}
#endif