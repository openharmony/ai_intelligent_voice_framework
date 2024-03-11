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

#ifndef ENROLL_INTELL_VOICE_ENGINE_NAPI_H
#define ENROLL_INTELL_VOICE_ENGINE_NAPI_H

#include <map>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "enroll_intell_voice_engine.h"
#include "enroll_intell_voice_engine_callback_napi.h"

namespace OHOS {
namespace IntellVoiceNapi {
using OHOS::IntellVoice::EnrollIntellVoiceEngine;
using OHOS::IntellVoice::EngineConfig;
using OHOS::IntellVoice::EnrollIntelligentVoiceEngineDescriptor;

class EnrollIntellVoiceEngineNapi {
public:
    EnrollIntellVoiceEngineNapi();
    ~EnrollIntellVoiceEngineNapi();

    static napi_value Export(napi_env env, napi_value exports);
    static napi_value CreateEnrollIntelligentVoiceEngineWrapper(napi_env env, AsyncContext *context);

private:
    static void Destruct(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static napi_value CreateEnrollIntelligentVoiceEngine(napi_env env, napi_callback_info info);

    static napi_value GetSupportedRegions(napi_env env, napi_callback_info info);
    static napi_value Init(napi_env env, napi_callback_info info);
    static napi_value EnrollForResult(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);
    static napi_value Commit(napi_env env, napi_callback_info info);
    static napi_value SetSensibility(napi_env env, napi_callback_info info);
    static napi_value SetWakeupHapInfo(napi_env env, napi_callback_info info);
    static napi_value SetParameter(napi_env env, napi_callback_info info);
    static napi_value GetParameter(napi_env env, napi_callback_info info);
    static napi_value EvaluateForResult(napi_env env, napi_callback_info info);
    static napi_value Release(napi_env env, napi_callback_info info);

    static void CompleteCallback(napi_env env, napi_status status, void *data);

private:
    napi_env env_ = nullptr;
    napi_ref wrapper_ = nullptr;
    static EnrollIntelligentVoiceEngineDescriptor g_enrollEngineDesc_;
    static int32_t constructResult_;
    std::shared_ptr<EnrollIntellVoiceEngine> engine_ = nullptr;
    std::shared_ptr<EnrollIntellVoiceEngineCallbackNapi> callbackNapi_ = nullptr;
    EngineConfig config_;
    bool isLast_ = false;
};
}  // namespace IntellVoiceNapi
}  // namespace OHOS

#endif
