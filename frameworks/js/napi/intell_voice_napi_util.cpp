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

#include "intell_voice_napi_util.h"
#include "intell_voice_log.h"

#define LOG_TAG "IntellVoiceNapiUtil"

using namespace std;

namespace OHOS {
namespace IntellVoiceNapi {
static const int32_t MAX_STRING_BUFFSIZE = 1024;

IntellVoiceRef::IntellVoiceRef(napi_env env, napi_value value)
{
    INTELL_VOICE_LOG_INFO("enter");
    env_ = env;
    napi_create_reference(env_, value, 1, &ref_);
}

IntellVoiceRef::~IntellVoiceRef()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (env_ != nullptr && ref_ != nullptr) {
        napi_delete_reference(env_, ref_);
    }
    env_ = nullptr;
    ref_ = nullptr;
}

napi_value IntellVoiceRef::GetRefValue()
{
    napi_value value = nullptr;
    napi_status status = napi_get_reference_value(env_, ref_, &value);
    if (status != napi_ok || value == nullptr) {
        INTELL_VOICE_LOG_ERROR("get reference value fail");
        return nullptr;
    }
    return value;
}

napi_value SetValue(napi_env env, const int32_t &value)
{
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, value, &result);
    if (status != napi_ok || result == nullptr) {
        INTELL_VOICE_LOG_ERROR("get js value fail");
        return nullptr;
    }
    return result;
}

napi_value SetValue(napi_env env, const string &value)
{
    napi_value result = nullptr;
    napi_status status = napi_create_string_utf8(env, value.c_str(), value.size(), &result);
    if (status != napi_ok || result == nullptr) {
        INTELL_VOICE_LOG_ERROR("get js value fail");
        return nullptr;
    }
    return result;
}

napi_status GetValue(napi_env env, napi_value jsValue, int32_t &value)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, jsValue, &valueType);
    if (valueType != napi_number) {
        return napi_generic_failure;
    }
    return napi_get_value_int32(env, jsValue, &value);
}

napi_status GetValue(napi_env env, napi_value jsValue, bool &value)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, jsValue, &valueType);
    if (valueType != napi_boolean) {
        return napi_generic_failure;
    }
    return napi_get_value_bool(env, jsValue, &value);
}

napi_status GetValue(napi_env env, napi_value jsValue, string &value)
{
    size_t bufferSize = 0;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, jsValue, &valueType);
    if (valueType != napi_string) {
        return napi_generic_failure;
    }
    napi_status status = napi_get_value_string_utf8(env, jsValue, nullptr, 0, &bufferSize);
    if (status != napi_ok || bufferSize == 0 || bufferSize >= MAX_STRING_BUFFSIZE) {
        INTELL_VOICE_LOG_ERROR("failed to get buffersize");
        return napi_generic_failure;
    }

    char *buffer = new (std::nothrow) char[(bufferSize + 1)];
    if (buffer == nullptr) {
        INTELL_VOICE_LOG_ERROR("no memory");
        return napi_generic_failure;
    }

    status = napi_get_value_string_utf8(env, jsValue, buffer, bufferSize + 1, &bufferSize);
    value = buffer;

    delete[] buffer;
    buffer = nullptr;
    return status;
}
}  // namespace IntellVoiceNapi
}  // namespace OHOS
