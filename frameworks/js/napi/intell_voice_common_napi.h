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

#ifndef INTELL_VOICE_COMMON_NAPI_H
#define INTELL_VOICE_COMMON_NAPI_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace IntellVoiceNapi {
constexpr int ARGC_ONE = 1;
constexpr int ARGC_TWO = 2;
constexpr int ARG_INDEX_0 = 0;
constexpr int ARG_INDEX_1 = 1;
constexpr int ARG_INDEX_2 = 2;

constexpr int32_t NAPI_INTELLIGENT_VOICE_SUCCESS = 0;

constexpr int32_t  NAPI_INTELLIGENT_VOICE_PERMISSION_DENIED = 201;
constexpr int32_t  NAPI_INTELLIGENT_VOICE_NO_MEMORY = 22700101;
constexpr int32_t  NAPI_INTELLIGENT_VOICE_INVALID_PARAM = 22700102;
constexpr int32_t  NAPI_INTELLIGENT_VOICE_INIT_FAILED = 22700103;
constexpr int32_t  NAPI_INTELLIGENT_VOICE_COMMIT_ENROLL_FAILED = 22700104;
constexpr int32_t  NAPI_INTELLIGENT_VOICE_START_CAPTURER_FAILED = 22700105;
constexpr int32_t  NAPI_INTELLIGENT_VOICE_READ_FAILED = 22700106;
constexpr int32_t  NAPI_INTELLIGENT_VOICE_SYSTEM_ERROR = 22700107;

class IntellVoiceCommonNapi {
public:
    IntellVoiceCommonNapi() = default;
    ~IntellVoiceCommonNapi() = default;
    static std::string GetMessageByCode(const int32_t code);
    static void ThrowError(napi_env env, int32_t code);
    static bool IsSameCallback(napi_env env, napi_value callback, napi_ref callbackRef);
};
}
}
#endif