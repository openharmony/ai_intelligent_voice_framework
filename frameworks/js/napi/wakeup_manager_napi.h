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
#ifndef WAKEUP_MANAGER_NAPI_H
#define WAKEUP_MANAGER_NAPI_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "intell_voice_manager.h"
#include "intell_voice_napi_queue.h"
#include "intell_voice_update_callback_napi.h"

namespace OHOS {
namespace IntellVoiceNapi {
class WakeupManagerNapi {
public:
    WakeupManagerNapi();
    ~WakeupManagerNapi();

    static napi_value Export(napi_env env, napi_value exports);

private:
    static void Destruct(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static napi_value GetWakeupManager(napi_env env, napi_callback_info info);

    static napi_value GetWakeupManagerWrapper(napi_env env);

    static napi_value SetParameter(napi_env env, napi_callback_info info);
    static napi_value GetParameter(napi_env env, napi_callback_info info);
    static napi_value GetUploadFiles(napi_env env, napi_callback_info info);
    static void GetUploadFilesComplete(napi_env env, AsyncContext *data, napi_value &result);
    static napi_value GetWakeupSourceFiles(napi_env env, napi_callback_info info);
    static void GetCloneCompleteCallback(napi_env env, AsyncContext *data, napi_value &result);
    static napi_value EnrollWithWakeupFilesForResult(napi_env env, napi_callback_info info);
    static void CloneForResultCompleteCallback(napi_env env, napi_status status, void *data);
    static bool WakeupFilesForResultParser(std::shared_ptr<UpdateAsyncContext> context,
        napi_env env, size_t argc, napi_value *argv);

private:
    napi_env env_ = nullptr;
    napi_ref wrapper_ = nullptr;
    static int32_t constructResult_;
    std::shared_ptr<IntellVoiceUpdateCallbackNapi> callbackNapi_ = nullptr;
};
}  // namespace IntellVoiceNapi
}  // namespace OHOS

#endif
