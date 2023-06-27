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

#include "intell_voice_napi_queue.h"
#include "intell_voice_napi_util.h"
#include "intell_voice_log.h"

#define LOG_TAG "IntellVoiceNapiQueue"

using namespace std;

namespace OHOS {
namespace IntellVoiceNapi {
AsyncComplete NapiAsync::complete_ = nullptr;

AsyncContext::AsyncContext()
{
    INTELL_VOICE_LOG_INFO("enter");
}
AsyncContext::~AsyncContext()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (env_ != nullptr) {
        if (work_ != nullptr) {
            napi_delete_async_work(env_, work_);
        }
        if (callbackRef_ != nullptr) {
            napi_delete_reference(env_, callbackRef_);
        }
        env_ = nullptr;
    }
}

void AsyncContext::GetCbInfo(napi_env env, napi_callback_info info, size_t callBackIndex, CbInfoParser parser)
{
    INTELL_VOICE_LOG_INFO("enter");
    napi_value jsThis = nullptr;
    const int32_t refCount = 1;
    size_t argc = ARGC_MAX;
    napi_value args[ARGC_MAX] = {nullptr};

    env_ = env;
    status_ = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_STATUS_RETURN_VOID(this, "Failed to get cb info");

    status_ = napi_unwrap(env, jsThis, &engineNapi_);
    CHECK_STATUS_RETURN_VOID(this, "Failed to get engine napi instance");

    if (argc > callBackIndex) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[callBackIndex], &valueType);
        if (valueType == napi_function) {
            status_ = napi_create_reference(env, args[callBackIndex], refCount, &callbackRef_);
            CHECK_STATUS_RETURN_VOID(this, "Failed to create callback reference");
        } else {
            INTELL_VOICE_LOG_ERROR("Failed to get asyncCallback, type mismatch");
        }
    }

    if (parser) {
        parser(argc, args);
    }
}

napi_value NapiAsync::AsyncWork(
    shared_ptr<AsyncContext> context, const string &name, AsyncExecute execute, AsyncComplete complete)
{
    INTELL_VOICE_LOG_INFO("enter");
    napi_value result = nullptr;
    if (context->callbackRef_ == nullptr) {
        napi_create_promise(context->env_, &context->deferred_, &result);
    } else {
        napi_get_undefined(context->env_, &result);
    }

    napi_value resource = nullptr;
    complete_ = complete;
    napi_create_string_utf8(context->env_, name.c_str(), NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(context->env_,
        nullptr,
        resource,
        execute,
        AsyncCompleteCallback,
        static_cast<void *>(context.get()),
        &context->work_);

    napi_queue_async_work(context->env_, context->work_);
    context->contextSp_ = context;

    return result;
}

napi_value NapiAsync::AsyncWork(
    shared_ptr<AsyncContext> context, const string &name, AsyncExecute execute, napi_async_complete_callback complete)
{
    INTELL_VOICE_LOG_INFO("enter");
    napi_value result = nullptr;
    if (context->callbackRef_ == nullptr) {
        napi_create_promise(context->env_, &context->deferred_, &result);
    } else {
        napi_get_undefined(context->env_, &result);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(context->env_, name.c_str(), NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(
        context->env_, nullptr, resource, execute, complete, static_cast<void *>(context.get()), &context->work_);

    napi_queue_async_work(context->env_, context->work_);
    context->contextSp_ = context;

    return result;
}

void NapiAsync::CommonCallbackRoutine(AsyncContext *context, const napi_value &data)
{
    CHECK_RETURN_VOID(context != nullptr, "async context is null");

    if (context->deferred_) {
        HandlePromise(context, data);
    } else {
        HandleAsyncCallback(context, data);
    }
    context->contextSp_.reset();
}

void NapiAsync::HandlePromise(AsyncContext *context, const napi_value &data)
{
    napi_value result = nullptr;
    napi_value error = nullptr;
    napi_get_undefined(context->env_, &result);
    napi_get_undefined(context->env_, &error);

    if (context->status_ == napi_ok) {
        result = data;
        napi_resolve_deferred(context->env_, context->deferred_, result);
        return;
    }
    napi_value message = nullptr;
    napi_create_string_utf8(context->env_, context->error_.c_str(), NAPI_AUTO_LENGTH, &message);
    napi_create_error(context->env_, nullptr, message, &error);
    napi_reject_deferred(context->env_, context->deferred_, error);
}

void NapiAsync::HandleAsyncCallback(AsyncContext *context, const napi_value &data)
{
    const size_t argc = 2;
    napi_value args[argc] = {nullptr, nullptr};
    napi_value callback = nullptr;
    napi_value ret = nullptr;
    if (context->status_ == napi_ok) {
        napi_get_undefined(context->env_, &args[0]);
        args[1] = data;
    } else {
        napi_value message = nullptr;
        napi_create_string_utf8(context->env_, context->error_.c_str(), NAPI_AUTO_LENGTH, &message);
        napi_create_error(context->env_, nullptr, message, &args[0]);
        napi_get_undefined(context->env_, &args[1]);
    }
    napi_get_reference_value(context->env_, context->callbackRef_, &callback);
    napi_call_function(context->env_, nullptr, callback, argc, args, &ret);
}

void NapiAsync::AsyncCompleteCallback(napi_env env, napi_status status, void *data)
{
    INTELL_VOICE_LOG_INFO("enter");
    CHECK_RETURN_VOID(data != nullptr, "async complete callback data is null");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    auto context = static_cast<AsyncContext *>(data);
    if ((status != napi_ok) && (context->status_ == napi_ok)) {
        context->status_ = status;
    }

    if (context->status_ == napi_ok && (complete_ != nullptr)) {
        complete_(context, result);
    }
    CommonCallbackRoutine(context, result);
}
}  // namespace IntellVoiceNapi
}  // namespace OHOS
