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

#ifndef INTELL_VOICE_NAPI_UTIL_H
#define INTELL_VOICE_NAPI_UTIL_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace IntellVoiceNapi {
#define CHECK_RETURN_VOID(condition, message) \
    do {                                      \
        if (!(condition)) {                   \
            INTELL_VOICE_LOG_ERROR(message);        \
            return;                           \
        }                                     \
    } while (0)

class IntellVoiceRef {
public:
    IntellVoiceRef(napi_env env, napi_value value);
    ~IntellVoiceRef();
    napi_value GetRefValue();

private:
    napi_env env_ = nullptr;
    napi_ref ref_ = nullptr;
};

napi_value SetValue(napi_env env, const int32_t &value);
napi_value SetValue(napi_env env, const std::string &value);

napi_status GetValue(napi_env env, napi_value jsValue, int32_t &value);
napi_status GetValue(napi_env env, napi_value jsValue, bool &value);
napi_status GetValue(napi_env env, napi_value jsValue, std::string &value);
}  // namespace IntellVoiceNapi
}  // namespace OHOS
#endif
