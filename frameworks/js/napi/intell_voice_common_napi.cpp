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
#include "intell_voice_common_napi.h"

#include <map>
#include "intell_voice_log.h"

#define LOG_TAG "IntellVoiceCommonNapi"

namespace OHOS {
namespace IntellVoiceNapi {
static const std::string NAPI_INTELLIGENT_VOICE_PERMISSION_DENIED_INFO = "Permission denied.";
static const std::string NAPI_INTELLIGENT_VOICE_NO_MEMORY_INFO = "No memory.";
static const std::string NAPI_INTELLIGENT_VOICE_INVALID_PARAM_INFO = "Input parameter value error.";
static const std::string NAPI_INTELLIGENT_VOICE_INIT_FAILED_INFO = "Init failed.";
static const std::string NAPI_INTELLIGENT_VOICE_COMMIT_ENROLL_FAILED_INFO = "Commit enroll failed.";
static const std::string NAPI_INTELLIGENT_VOICE_START_CAPTURER_INFO = "Start capturer failed.";
static const std::string NAPI_INTELLIGENT_VOICE_READ_FAILED_INFO = "Read failed.";
static const std::string NAPI_INTELLIGENT_VOICE_SYSTEM_ERROR_INFO = "System error.";

std::string IntellVoiceCommonNapi::GetMessageByCode(int32_t code)
{
    static const std::map<int32_t, std::string> messageMap = {
        {NAPI_INTELLIGENT_VOICE_PERMISSION_DENIED, NAPI_INTELLIGENT_VOICE_PERMISSION_DENIED_INFO},
        {NAPI_INTELLIGENT_VOICE_NO_MEMORY, NAPI_INTELLIGENT_VOICE_NO_MEMORY_INFO},
        {NAPI_INTELLIGENT_VOICE_INVALID_PARAM, NAPI_INTELLIGENT_VOICE_INVALID_PARAM_INFO},
        {NAPI_INTELLIGENT_VOICE_INIT_FAILED, NAPI_INTELLIGENT_VOICE_INIT_FAILED_INFO},
        {NAPI_INTELLIGENT_VOICE_COMMIT_ENROLL_FAILED, NAPI_INTELLIGENT_VOICE_COMMIT_ENROLL_FAILED_INFO},
        {NAPI_INTELLIGENT_VOICE_START_CAPTURER_FAILED, NAPI_INTELLIGENT_VOICE_START_CAPTURER_INFO},
        {NAPI_INTELLIGENT_VOICE_READ_FAILED, NAPI_INTELLIGENT_VOICE_READ_FAILED_INFO},
        {NAPI_INTELLIGENT_VOICE_SYSTEM_ERROR, NAPI_INTELLIGENT_VOICE_SYSTEM_ERROR_INFO}
    };

    auto iter = messageMap.find(code);
    if (iter == messageMap.end()) {
        INTELL_VOICE_LOG_ERROR("invalid code: %{public}d", code);
        return "";
    }

    return iter->second;
}

void IntellVoiceCommonNapi::ThrowError(napi_env env, int32_t code)
{
    std::string messageValue = GetMessageByCode(code);
    napi_throw_error(env, (std::to_string(code)).c_str(), messageValue.c_str());
}

bool IntellVoiceCommonNapi::IsSameCallback(napi_env env, napi_value callback, napi_ref callbackRef)
{
    bool isEqual = false;
    napi_value copyValue = nullptr;

    napi_get_reference_value(env, callbackRef, &copyValue);
    napi_status status = napi_strict_equals(env, copyValue, callback, &isEqual);
    if (status != napi_ok) {
        INTELL_VOICE_LOG_ERROR("get napi_strict_equals failed, status:%{public}d", status);
        return false;
    }

    return isEqual;
}
}
}
