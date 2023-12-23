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
#ifndef INTELL_VOICE_TRIGGER_MANAGER_H
#define INTELL_VOICE_TRIGGER_MANAGER_H

#include <map>
#include <memory>
#include "trigger_base_type.h"
#include "trigger_service.h"
#include "trigger_detector.h"
#include "i_intell_voice_trigger_detector_callback.h"

namespace OHOS {
namespace IntellVoiceTrigger {
class TriggerManager {
public:
    ~TriggerManager();
    static std::shared_ptr<TriggerManager> GetInstance();

    void UpdateModel(std::shared_ptr<GenericTriggerModel> model);
    void DeleteModel(int32_t uuid);
    std::shared_ptr<GenericTriggerModel> GetModel(int32_t uuid);
    std::shared_ptr<TriggerDetector> CreateTriggerDetector(
        int32_t uuid, std::shared_ptr<IIntellVoiceTriggerDetectorCallback> callback);
    void ReleaseTriggerDetector(int32_t uuid);
    int32_t SetParameter(const std::string &key, const std::string &value);
    std::string GetParameter(const std::string &key);
    void AttachTelephonyObserver();
    void DetachTelephonyObserver();
    void AttachAudioCaptureListener();
    void DetachAudioCaptureListener();
    void AttachAudioRendererEventListener();
    void DetachAudioRendererEventListener();

private:
    explicit TriggerManager();

private:
    std::map<int32_t, std::shared_ptr<TriggerDetector>> detectors_;
    std::shared_ptr<TriggerService> service_;

    static std::mutex instanceMutex_;
    static std::shared_ptr<TriggerManager> instance_;
};
}  // namespace IntellVoiceTrigger
}  // namespace OHOS
#endif