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

#ifndef INTELL_VOICE_UTILS_H
#define INTELL_VOICE_UTILS_H

#include <string>

namespace OHOS {
namespace IntellVoice {
enum ServiceChangeType {
    /* Service unavailable. */
    SERVICE_UNAVAILABLE = 0,
};

enum IntelligentVoiceEngineType {
    /* Enroll engine. */
    ENROLL_ENGINE_TYPE = 0,
    /* Wakeup engine. */
    WAKEUP_ENGINE_TYPE = 1,
    /* Update engine. */
    UPDATE_ENGINE_TYPE = 2,
};

enum SensibilityType {
    /* Low sensibility. */
    LOW_SENSIBILITY = 1,
    /* Middle sensibility. */
    MIDDLE_SENSIBILITY = 2,
    /* High sensibility. */
    HIGH_SENSIBILITY = 3,
};

enum EnrollIntelligentVoiceEventType {
    /* Enroll None. */
    INTELLIGENT_VOICE_EVENT_ENROLL_NONE = 0,
    /* Init done. */
    INTELLIGENT_VOICE_EVENT_ENROLL_INIT_DONE = 1,
    /* Enroll complete. */
    INTELLIGENT_VOICE_EVENT_ENROLL_COMPLETE = 2,
    /* Commit enroll complete. */
    INTELLIGENT_VOICE_EVENT_COMMIT_ENROLL_COMPLETE = 3,
};

enum WakeupIntelligentVoiceEventType {
    /* Wakeup None. */
    INTELLIGENT_VOICE_EVENT_WAKEUP_NONE = 0,
    /* Recognize complete. */
    INTELLIGENT_VOICE_EVENT_RECOGNIZE_COMPLETE = 1,
};

enum IntelligentVoiceErrorCode {
    /* Success. */
    INTELLIGENT_VOICE_SUCCESS = 0,
    /* Memory is insufficient. */
    INTELLIGENT_VOICE_NO_MEMORY = 22700101,
    /* Invalid parameter. */
    INTELLIGENT_VOICE_INVALID_PARAM = 22700102,
    /* Init failed. */
    INTELLIGENT_VOICE_INIT_FAILED = 22700103,
    /* Enroll failed. */
    INTELLIGENT_VOICE_ENROLL_FAILED = 22700104,
    /* Commit enroll failed. */
    INTELLIGENT_VOICE_COMMIT_ENROLL_FAILED = 22700105,
    /* Recognize failed. */
    INTELLIGENT_VOICE_RECOGNIZE_FAILED = 22700106,
};

struct WakeupHapInfo {
    std::string bundleName;
    std::string abilityName;
};
}  // namespace IntellVoice
}  // namespace OHOS
#endif
