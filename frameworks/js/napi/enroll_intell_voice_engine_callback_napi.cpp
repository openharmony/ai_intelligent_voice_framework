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

#include "enroll_intell_voice_engine_callback_napi.h"
#include "intell_voice_log.h"

using namespace std;
using namespace OHOS::IntellVoiceEngine;
#define LOG_TAG "EnrollIntellVoiceEngineCallbackNapi"

namespace OHOS {
namespace IntellVoiceNapi {
void EnrollCallbackInfo::GetCallBackInfoNapiValue(const napi_env &env, napi_value &out)
{
    napi_status status = napi_create_object(env, &out);
    if (status != napi_ok || out == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to create js callbackInfo, error: %{public}d", status);
        return;
    }

    napi_set_named_property(env, out, "result", SetValue(env, result));
    napi_set_named_property(env, out, "context", SetValue(env, context));
}

EnrollIntellVoiceEngineCallbackNapi::EnrollIntellVoiceEngineCallbackNapi(const napi_env env) : env_(env)
{
    if (env_ != nullptr) {
        napi_get_uv_event_loop(env_, &loop_);
    }
    contextMap_.clear();
}

EnrollIntellVoiceEngineCallbackNapi::~EnrollIntellVoiceEngineCallbackNapi()
{
    contextMap_.clear();
}

void EnrollIntellVoiceEngineCallbackNapi::QueueAsyncWork(EnrollAsyncContext *context)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (contextMap_.find(context->type) == contextMap_.end()) {
        std::queue<EnrollAsyncContext *> contextQue;
        contextQue.push(context);
        contextMap_[context->type] = contextQue;
    } else {
        contextMap_.at(context->type).push(context);
    }
}

void EnrollIntellVoiceEngineCallbackNapi::ClearAsyncWork(bool error, const std::string &msg)
{
    INTELL_VOICE_LOG_INFO("%{public}s", msg.c_str());
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = contextMap_.begin(); it != contextMap_.end(); it++) {
        auto &contextQue = it->second;
        while (!contextQue.empty()) {
            EnrollAsyncContext *context = contextQue.front();
            contextQue.pop();
            if (error) {
                INTELL_VOICE_LOG_WARN("error occured");
            }
            OnJsCallBack(context);
        }
    }
    contextMap_.clear();
}

void EnrollIntellVoiceEngineCallbackNapi::OnEvent(const IntellVoiceEngineCallBackEvent &event)
{
    INTELL_VOICE_LOG_INFO("OnEvent: msgId: %{public}d, errCode: %{public}d, context: %{public}s",
        event.msgId,
        event.result,
        event.info.c_str());
    EnrollAsyncWorkType asyncType = ASYNC_WORK_INVALID;
    switch (event.msgId) {
        case HDI::IntelligentVoice::Engine::V1_0::INTELL_VOICE_ENGINE_MSG_INIT_DONE:
            asyncType = ASYNC_WORK_INIT;
            break;
        case HDI::IntelligentVoice::Engine::V1_0::INTELL_VOICE_ENGINE_MSG_ENROLL_COMPLETE:
            asyncType = ASYNC_WORK_START;
            break;
        case HDI::IntelligentVoice::Engine::V1_0::INTELL_VOICE_ENGINE_MSG_COMMIT_ENROLL_COMPLETE:
            asyncType = ASYNC_WORK_COMMIT;
            break;
        default:
            break;
    }
    if (contextMap_.find(asyncType) == contextMap_.end() || contextMap_.at(asyncType).empty()) {
        INTELL_VOICE_LOG_ERROR("callback is called, But context is empty");
        return;
    }

    EnrollAsyncContext *context = contextMap_.at(asyncType).front();
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("context is nullptr");
        return;
    }
    contextMap_.at(asyncType).pop();

    context->callbackInfo = {event.msgId, event.result, event.info};
    OnJsCallBack(context);
}

void EnrollIntellVoiceEngineCallbackNapi::UvWorkCallBack(uv_work_t *work, int status)
{
    INTELL_VOICE_LOG_INFO("enter");
    auto asyncContext = reinterpret_cast<EnrollAsyncContext *>(work->data);
    napi_value result = nullptr;
    if (asyncContext != nullptr) {
        napi_env env = asyncContext->env_;
        if (asyncContext->callbackInfo.eventId ==
            HDI::IntelligentVoice::Engine::V1_0::INTELL_VOICE_ENGINE_MSG_ENROLL_COMPLETE) {
            asyncContext->callbackInfo.GetCallBackInfoNapiValue(env, result);
        } else {
            napi_get_undefined(env, &result);
        }
        NapiAsync::CommonCallbackRoutine(env, asyncContext, result);
    }

    delete work;
}

void EnrollIntellVoiceEngineCallbackNapi::OnJsCallBack(EnrollAsyncContext *context)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (loop_ != nullptr) {
        uv_work_t *work = new (std::nothrow) uv_work_t;
        if (work != nullptr) {
            work->data = reinterpret_cast<void *>(context);
            int ret = uv_queue_work(
                loop_, work, [](uv_work_t *work) {}, UvWorkCallBack);
            if (ret != 0) {
                INTELL_VOICE_LOG_INFO("Failed to execute libuv work queue");
                delete context;
                delete work;
            }
        } else {
            delete context;
        }
    } else {
        delete context;
    }
}
}  // namespace IntellVoiceNapi
}  // namespace OHOS