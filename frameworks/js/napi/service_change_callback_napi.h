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

#ifndef SERVICE_CHANGE_CALLBACK_H
#define SERVICE_CHANGE_CALLBACK_H

#include <mutex>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "iremote_object.h"
#include "intell_voice_napi_util.h"

namespace OHOS {
namespace IntellVoiceNapi {
class ServiceChangeCallbackNapi : public IRemoteObject::DeathRecipient {
public:
    explicit ServiceChangeCallbackNapi(napi_env env);
    ~ServiceChangeCallbackNapi() override {};
    void SaveCallbackReference(napi_value callback);
    void OnRemoteDied(const wptr <IRemoteObject> &remote) override;

private:
    struct ServiceChangeCallbackData {
        int32_t status;
        std::shared_ptr<IntellVoiceRef> callback;
    };

    void OnJsCallbackServiceChange(int32_t status);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    uv_loop_s *loop_ = nullptr;
    std::shared_ptr<IntellVoiceRef> callbackRef_ = nullptr;
};
}  // namespace IntellVoiceNapi
}  // namespace OHOS
#endif
