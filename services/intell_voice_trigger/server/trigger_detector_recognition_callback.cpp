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

#include "trigger_detector_recognition_callback.h"
#include "intell_voice_log.h"

#define LOG_TAG "TriggerDetectorRecogCallback"

namespace OHOS {
namespace IntellVoiceTrigger {
TriggerDetectorRecognitionCallback::TriggerDetectorRecognitionCallback(
    std::shared_ptr<IIntellVoiceTriggerDetectorCallback> callback) : callback_(callback)
{
}

void TriggerDetectorRecognitionCallback::OnGenericTriggerDetected(
    const std::shared_ptr<GenericTriggerEvent> event)
{
    if (event == nullptr || callback_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("event or callback_ is nullptr");
        return;
    }

    std::shared_ptr<DetectorEvent> eventPayLoad = std::make_shared<DetectorEvent>(
        event->audioFormat_, event->data_);
    if (eventPayLoad == nullptr) {
        INTELL_VOICE_LOG_ERROR("event or callback_ is nullptr");
        return;
    }

    callback_->OnDetected(eventPayLoad);
}
}
}