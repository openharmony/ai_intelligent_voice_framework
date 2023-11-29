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

#ifndef ENROLL_INTELL_VOICE_ENGINE_CALLBACK_NAPI_H
#define ENROLL_INTELL_VOICE_ENGINE_CALLBACK_NAPI_H

#include <queue>
#include <map>
#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "intell_voice_napi_util.h"
#include "i_intell_voice_engine_callback.h"
#include "intell_voice_napi_queue.h"

namespace OHOS {
namespace IntellVoiceNapi {
using OHOS::IntellVoiceEngine::IIntellVoiceEngineEventCallback;
using OHOS::IntellVoiceEngine::IntellVoiceEngineCallBackEvent;

using ProcessWorkFunc = std::function<int32_t(AsyncContext *)>;

enum EnrollAsyncWorkType {
    ASYNC_WORK_INIT = 0,
    ASYNC_WORK_START,
    ASYNC_WORK_COMMIT,
    ASYNC_WORK_INVALID,
};

struct EnrollCallbackInfo {
    int32_t eventId;
    int32_t result;
    std::string context;

    void GetCallBackInfoNapiValue(const napi_env &env, napi_value &out);
};

class EnrollAsyncContext : public AsyncContext {
public:
    explicit EnrollAsyncContext(napi_env env) : AsyncContext(env) {};
    EnrollAsyncWorkType type = ASYNC_WORK_INVALID;
    ProcessWorkFunc processWork = nullptr;
    EnrollCallbackInfo callbackInfo;
};

class EnrollIntellVoiceEngineCallbackNapi : public IIntellVoiceEngineEventCallback {
public:
    explicit EnrollIntellVoiceEngineCallbackNapi(const napi_env env);
    virtual ~EnrollIntellVoiceEngineCallbackNapi();

    void OnEvent(const IntellVoiceEngineCallBackEvent &event) override;
    void QueueAsyncWork(EnrollAsyncContext *context);
    void ClearAsyncWork(bool error, const std::string &msg);

private:
    int32_t ConvertEventId(EnrollAsyncWorkType type);
    void OnJsCallBack(EnrollAsyncContext *context);
    static void UvWorkCallBack(uv_work_t *work, int status);

private:
    std::mutex mutex_;
    napi_env env_ = nullptr;
    uv_loop_s *loop_ = nullptr;

    std::map<EnrollAsyncWorkType, std::queue<EnrollAsyncContext *>> contextMap_;
};
}  // namespace IntellVoiceNapi
}  // namespace OHOS
#endif
