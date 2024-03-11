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

#ifndef INTELL_VOICE_NAPI_QUEUE_H
#define INTELL_VOICE_NAPI_QUEUE_H

#include <functional>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace IntellVoiceNapi {
constexpr size_t ARGC_MAX = 4;

class AsyncContext;
using CbInfoParser = std::function<bool(size_t argc, napi_value *argv)>;
using AsyncExecute = napi_async_execute_callback;
using AsyncComplete = std::function<void(napi_env, AsyncContext *, napi_value &)>;

class AsyncContext {
public:
    explicit AsyncContext(napi_env env);
    ~AsyncContext();
    bool GetCbInfo(napi_env env, napi_callback_info info, size_t callBackIndex, CbInfoParser parser);

    napi_env env_ = nullptr;
    void *instanceNapi_ = nullptr;

    int32_t result_ = -1;
    AsyncComplete complete_ = nullptr;

    napi_async_work work_ = nullptr;
    napi_deferred deferred_ = nullptr;
    napi_ref callbackRef_ = nullptr;
    std::shared_ptr<AsyncContext> contextSp_ = nullptr;
};

class NapiAsync {
public:
    static napi_value AsyncWork(napi_env env, std::shared_ptr<AsyncContext> context, const std::string &name,
        AsyncExecute execute);
    static napi_value AsyncWork(napi_env env, std::shared_ptr<AsyncContext> context, const std::string &name,
        AsyncExecute execute, napi_async_complete_callback complete);
    static void CommonCallbackRoutine(napi_env env, AsyncContext *context, const napi_value &data);

private:
    static void AsyncCompleteCallback(napi_env env, napi_status status, void *data);
    static void HandlePromise(napi_env env, AsyncContext *context, const napi_value &data);
    static void HandleAsyncCallback(napi_env env, AsyncContext *context, const napi_value &data);
};
}  // namespace IntellVoiceNapi
}  // namespace OHOS

#endif
