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

#ifndef UV_CALLBACK_NAPI_H
#define UV_CALLBACK_NAPI_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "intell_voice_napi_util.h"
#include "i_intell_voice_engine_callback.h"

namespace OHOS {
namespace IntellVoiceNapi {
using OHOS::IntellVoiceEngine::IIntellVoiceEngineEventCallback;
using OHOS::IntellVoiceEngine::IntellVoiceEngineCallBackEvent;

class UvCallbackNapi {
public:
    explicit UvCallbackNapi(napi_env env, napi_value callback);
    virtual ~UvCallbackNapi() = default;

    void OnUvCallback(napi_value &cbInfo);

private:
    struct UvCallbackData {
        napi_env env_ = nullptr;
        napi_value cbInfo;
        std::shared_ptr<IntellVoiceRef> callback;
    };

protected:
    napi_env env_ = nullptr;
    uv_loop_s *loop_ = nullptr;
    std::shared_ptr<IntellVoiceRef> callbackRef_ = nullptr;
};
}  // namespace IntellVoiceNapi
}  // namespace OHOS
#endif
