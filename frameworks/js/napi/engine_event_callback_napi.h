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

#ifndef ENGINE_EVENT_CALLBACK_H
#define ENGINE_EVENT_CALLBACK_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "intell_voice_napi_util.h"
#include "i_intell_voice_engine_callback.h"
#include "uv_callback_napi.h"

namespace OHOS {
namespace IntellVoiceNapi {
using OHOS::IntellVoiceEngine::IIntellVoiceEngineEventCallback;
using OHOS::IntellVoiceEngine::IntellVoiceEngineCallBackEvent;

struct EngineCallBackInfo {
    int32_t eventId;
    bool isSuccess;
    std::string context;
};

class EngineEventCallbackNapi : public IIntellVoiceEngineEventCallback, UvCallbackNapi {
public:
    explicit EngineEventCallbackNapi(napi_env env, napi_value callback) : UvCallbackNapi(env, callback){};
    ~EngineEventCallbackNapi() override {};

    void OnEvent(const IntellVoiceEngineCallBackEvent &event) override;
    napi_value GetCallBackInfoNapiValue(const EngineCallBackInfo &callbackInfo);
};
}  // namespace IntellVoiceNapi
}  // namespace OHOS
#endif
