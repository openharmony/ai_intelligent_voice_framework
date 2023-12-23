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
#ifndef INTELL_VOICE_TRIGGER_SERVICE_H
#define INTELL_VOICE_TRIGGER_SERVICE_H

#include "trigger_db_helper.h"
#include "trigger_helper.h"

namespace OHOS {
namespace IntellVoiceTrigger {
class TriggerService {
public:
    TriggerService();
    ~TriggerService();
    void UpdateGenericTriggerModel(std::shared_ptr<GenericTriggerModel> model);
    void DeleteGenericTriggerModel(int32_t uuid);
    std::shared_ptr<GenericTriggerModel> GetGenericTriggerModel(int32_t uuid);

    int32_t StartRecognition(int32_t uuid, std::shared_ptr<IIntellVoiceTriggerRecognitionCallback> callback);
    int32_t StopRecognition(int32_t uuid, std::shared_ptr<IIntellVoiceTriggerRecognitionCallback> callback);
    void UnloadTriggerModel(int32_t uuid);
    int32_t SetParameter(const std::string &key, const std::string &value);
    std::string GetParameter(const std::string &key);
    void AttachTelephonyObserver();
    void DetachTelephonyObserver();
    void AttachAudioCaptureListener();
    void DetachAudioCaptureListener();
    void AttachAudioRendererEventListener();
    void DetachAudioRendererEventListener();

private:
    std::shared_ptr<TriggerDbHelper> dbHelper_ = nullptr;
    std::shared_ptr<TriggerHelper> triggerHelper_ = nullptr;
};
}  // namespace IntellVoiceTrigger
}  // namespace OHOS
#endif