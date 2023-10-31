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
#include "intell_voice_info.h"
#include "intell_voice_log.h"
#include "intell_voice_napi_util.h"
#include "intell_voice_info.h"
#include "service_change_callback_napi.h"

#define LOG_TAG "ServiceChangeCallbackNapi"

using namespace std;
using namespace OHOS::IntellVoice;

namespace OHOS {
namespace IntellVoiceNapi {
void ServiceChangeCallbackNapi::OnRemoteDied(const wptr <IRemoteObject> &remote)
{
    INTELL_VOICE_LOG_INFO("receive sa death callback");
    (void)remote;
    int32_t status = ServiceChangeType::SERVICE_UNAVAILABLE;

    napi_value jsCbInfo = SetValue(env_, status);
    return OnUvCallback(jsCbInfo);
}
}  // namespace IntellVoiceNapi
}  // namespace OHOS
