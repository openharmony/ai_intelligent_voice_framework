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
#include "trigger_callback_impl.h"
#include "intell_voice_log.h"

using namespace OHOS::HDI::IntelligentVoice::Trigger::V1_0;

#define LOG_TAG "TriggerCallbackImpl"

namespace OHOS {
namespace IntellVoiceTrigger {
TriggerCallbackImpl::TriggerCallbackImpl(std::shared_ptr<IIntellVoiceTriggerAdapterListener> listener)
    :listener_(listener)
{
}

int32_t TriggerCallbackImpl::OnRecognitionHdiEvent(const IntellVoiceRecognitionEvent &event, int32_t cookie)
{
    if (listener_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("listener_ is nullptr");
        return -1;
    }

    listener_->OnRecognitionHdiEvent(event, cookie);
    return 0;
}
}
}