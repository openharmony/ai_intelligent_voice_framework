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
#ifndef INTELL_VOICE_TRIGGER_DETECTOR_H
#define INTELL_VOICE_TRIGGER_DETECTOR_H

#include <memory>
#include "trigger_service.h"
#include "i_intell_voice_trigger_detector_callback.h"
#include "i_intell_voice_trigger_recognition_callback.h"

namespace OHOS {
namespace IntellVoiceTrigger {
class TriggerDetector {
public:
    TriggerDetector(int32_t uuid, std::shared_ptr<TriggerService> service,
        std::shared_ptr<IIntellVoiceTriggerDetectorCallback> callback);
    ~TriggerDetector();
    bool StartRecognition();
    bool StopRecognition();
    void UnloadTriggerModel();

private:
    int32_t uuid_ = -1;
    std::shared_ptr<TriggerService> service_ = nullptr;
    std::shared_ptr<IIntellVoiceTriggerRecognitionCallback> callback_ = nullptr;
};
}
}
#endif