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

enum EnrollResult {
    /* Enroll success. */
    SUCCESS = 0,
    /* Vpr train failed. */
    VPR_TRAIN_FAILED = -1,
    /* Wakeup phrase not match. */
    WAKEUP_PHRASE_NOT_MATCH = -2,
    /* Too noisy. */
    TOO_NOISY = -3,
    /* Too loud. */
    TOO_LOUD = -4,
    /* Interval large. */
    INTERVAL_LARGE = -5,
    /* Different person. */
    DIFFERENT_PERSON = -6,
    /* Unknown error. */
    UNKNOWN_ERROR = -100,
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
    /* Reconfirm Recognition complete. */
    INTELLIGENT_VOICE_EVENT_RECONFIRM_RECOGNITION_COMPLETE = 2,
    /* Headset Recognize complete. */
    INTELLIGENT_VOICE_EVENT_HEADSET_RECOGNIZE_COMPLETE = 3,
};

enum IntelligentVoiceErrorCode {
    /* No memory. */
    INTELLIGENT_VOICE_NO_MEMORY = 22700101,
    /* Input parameter value error. */
    INTELLIGENT_VOICE_INVALID_PARAM = 22700102,
    /* Init failed. */
    INTELLIGENT_VOICE_INIT_FAILED = 22700103,
    /* Commit enroll failed. */
    INTELLIGENT_VOICE_COMMIT_ENROLL_FAILED = 22700104,
    /* Start capturer failed. */
    INTELLIGENT_VOICE_START_CAPTURER_FAILED = 22700105,
    /* Read failed. */
    INTELLIGENT_VOICE_READ_FAILED = 22700106,
    /* System error. */
    INTELLIGENT_VOICE_SYSTEM_ERROR = 22700107,
};

enum UploadFileType {
    /* Enroll file. */
    ENROLL_FILE = 0,
    /* Wakeup file. */
    WAKEUP_FILE = 1,
};

enum EvaluationResultCode {
    /* Unknown */
    UNKNOWN = 0,
    /* Pass */
    PASS = 1,
    /* Word is empty */
    WORD_EMPTY = 2,
    /* Only chinese is supported */
    CHINESE_ONLY = 3,
    /* Invalid length */
    INVALID_LENGTH = 4,
    /* Unusual word */
    UNUSUAL_WORD = 5,
    /* Consecutive same word */
    CONSECUTIVE_SAME_WORD = 6,
    /* Too few phonemes */
    TOO_FEW_PHONEMES = 7,
    /* Too many phoneme */
    TOO_MANY_PHONEMES = 8,
    /* Contain common instruction */
    COMMON_INSTRUCTION = 9,
    /* Contain common spoken language */
    COMMON_SPOKEN_LANGUAGE = 10,
    /* Contain sensitive word */
    SENSITIVE_WORD = 11,
    /* Two consecutive words without initial consonant */
    NO_INITIAL_CONSONANT = 12,
    /* Contain repeated phoneme */
    REPEATED_PHONEME = 13,
};

struct WakeupHapInfo {
    std::string bundleName;
    std::string abilityName;
};

struct WakeupSourceFile {
    std::string filePath;
    std::vector<uint8_t> fileContent;
};

}  // namespace IntellVoice
}  // namespace OHOS
#endif
