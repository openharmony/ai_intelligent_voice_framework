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
#include "trigger_detector_callback.h"
#include <thread>
#include "intell_voice_log.h"
#include "wakeup_engine.h"

using namespace OHOS::IntellVoiceEngine;
#define LOG_TAG "TriggerDetectorCallback"

namespace OHOS {
namespace IntellVoiceTrigger {
TriggerDetectorCallback::TriggerDetectorCallback(OnDetectedCb cb) : cb_(cb)
{}

void TriggerDetectorCallback::OnDetected(const std::shared_ptr<DetectorEvent> &event)
{
    if ((event == nullptr) || (cb_ == nullptr)) {
        INTELL_VOICE_LOG_ERROR("event or cb is nullptr");
        return;
    }

    if (event->data_.size() > 0) {
        INTELL_VOICE_LOG_INFO(
            "receive DetectorEvent dataSize_: %{public}zu, data_[0]: %{public}d", event->data_.size(), event->data_[0]);
    }

    std::thread([&]() { cb_(); }).detach();
}
}  // namespace IntellVoiceTrigger
}  // namespace OHOS