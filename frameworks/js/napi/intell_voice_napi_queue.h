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

using CbInfoParser = std::function<void(size_t argc, napi_value *argv)>;

#define CHECK_STATUS_RETURN_VOID(context, message) \
    do {                                           \
        if ((context)->status_ != napi_ok) {       \
            (context)->error_ = std::string(message);   \
            INTELL_VOICE_LOG_ERROR(message);       \
            return;                                \
        }                                          \
    } while (0)

class AsyncContext {
public:
    AsyncContext();
    ~AsyncContext();
    void GetCbInfo(napi_env env, napi_callback_info info, size_t callBackIndex, CbInfoParser parser);

    napi_env env_ = nullptr;
    void *engineNapi_ = nullptr;

    napi_status status_ = napi_invalid_arg;
    std::string error_;
    int32_t result_ = -1;

private:
    napi_async_work work_ = nullptr;
    napi_deferred deferred_ = nullptr;
    napi_ref callbackRef_ = nullptr;
    std::shared_ptr<AsyncContext> contextSp_ = nullptr;

    friend class NapiAsync;
};

using AsyncExecute = napi_async_execute_callback;
using AsyncComplete = std::function<void(AsyncContext *, napi_value &)>;

class NapiAsync {
public:
    static napi_value AsyncWork(std::shared_ptr<AsyncContext> context, const std::string &name, AsyncExecute execute,
        AsyncComplete complete = nullptr);
    static napi_value AsyncWork(std::shared_ptr<AsyncContext> context, const std::string &name, AsyncExecute execute,
        napi_async_complete_callback complete);
    static void CommonCallbackRoutine(AsyncContext *context, const napi_value &data);

private:
    static void AsyncCompleteCallback(napi_env env, napi_status status, void *data);
    static void HandlePromise(AsyncContext *context, const napi_value &data);
    static void HandleAsyncCallback(AsyncContext *context, const napi_value &data);

    static AsyncComplete complete_;
};
}  // namespace IntellVoiceNapi
}  // namespace OHOS

#endif
