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
#include <uv.h>
#include "intell_voice_log.h"
#include "uv_callback_napi.h"

#define LOG_TAG "UvCallbackNapi"

using namespace std;

namespace OHOS {
namespace IntellVoiceNapi {
UvCallbackNapi::UvCallbackNapi(napi_env env, napi_value callback)
{
    INTELL_VOICE_LOG_INFO("enter");
    env_ = env;
    if (env_ != nullptr) {
        napi_get_uv_event_loop(env_, &loop_);
    }
    callbackRef_ = make_shared<IntellVoiceRef>(env_, callback);
}

void UvCallbackNapi::OnUvCallback(napi_value &cbInfo)
{
    CHECK_RETURN_VOID(loop_ != nullptr, "loop is nullptr");
    uv_work_t *work = new (nothrow) uv_work_t;
    CHECK_RETURN_VOID(work != nullptr, "Create uv work failed, no memory");
    CHECK_RETURN_VOID(cbInfo != nullptr, "cbInfo is nullptr");

    work->data = new UvCallbackData {env_, cbInfo, callbackRef_};

    uv_queue_work(
        loop_,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int uvStatus) {
            shared_ptr<UvCallbackData> uvCallback(
                static_cast<UvCallbackData *>(work->data), [work](UvCallbackData *data) {
                    delete data;
                    delete work;
                });
            CHECK_RETURN_VOID(uvCallback != nullptr, "uvCallback is nullptr");
            CHECK_RETURN_VOID(uvCallback->callback != nullptr, "uvCallback callback is nullptr");
            CHECK_RETURN_VOID(uvCallback->cbInfo != nullptr, "uvCallback callback info is nullptr");
            napi_env env = uvCallback->env_;
            INTELL_VOICE_LOG_INFO("uv_queue_work start");

            napi_value jsCallback = uvCallback->callback->GetRefValue();
            if (jsCallback == nullptr) {
                INTELL_VOICE_LOG_ERROR("get reference value fail");
                return;
            }

            const size_t argc = 1;
            napi_value args[argc] = {uvCallback->cbInfo};
            napi_value result = nullptr;
            napi_status status = napi_call_function(env, nullptr, jsCallback, argc, args, &result);
            if (status != napi_ok) {
                INTELL_VOICE_LOG_ERROR("failed to call engine event callback, error: %{public}d", status);
            }
        });
}
}  // namespace IntellVoiceNapi
}  // namespace OHOS
