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
#include "engine_event_callback_napi.h"
#include <uv.h>
#include "intell_voice_info.h"
#include "intell_voice_log.h"

#define LOG_TAG "EngineEventCallbackNapi"

using namespace std;
using namespace OHOS::IntellVoice;
using namespace OHOS::IntellVoiceEngine;

namespace OHOS {
namespace IntellVoiceNapi {
void EngineEventCallbackNapi::OnEvent(const IntellVoiceEngineCallBackEvent &event)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (event.msgId != HDI::IntelligentVoice::Engine::V1_0::INTELL_VOICE_ENGINE_MSG_RECOGNIZE_COMPLETE) {
        INTELL_VOICE_LOG_ERROR("error msgId");
        return;
    }

    EngineCallBackInfo cbInfo = {INTELLIGENT_VOICE_EVENT_RECOGNIZE_COMPLETE,
        event.result == 0 ? true : false, event.info};

    INTELL_VOICE_LOG_INFO("OnEvent EngineCallBackInfo: eventId: %{public}d, isSuccess: %{public}u, context: %{public}s",
        cbInfo.eventId,
        cbInfo.isSuccess,
        cbInfo.context.c_str());
    napi_value jsCbInfo = GetCallBackInfoNapiValue(cbInfo);
    return OnUvCallback(jsCbInfo);
}

napi_value EngineEventCallbackNapi::GetCallBackInfoNapiValue(const EngineCallBackInfo &callbackInfo)
{
    napi_value result;
    napi_status status = napi_create_object(env_, &result);
    if (status != napi_ok || result == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to create js callbackInfo, error: %{public}d", status);
        return nullptr;
    }

    napi_set_named_property(env_, result, "eventId", SetValue(env_, callbackInfo.eventId));
    napi_set_named_property(env_, result, "isSuccess", SetValue(env_, callbackInfo.isSuccess));
    napi_set_named_property(env_, result, "context", SetValue(env_, callbackInfo.context));
    return result;
}
}  // namespace IntellVoiceNapi
}  // namespace OHOS
