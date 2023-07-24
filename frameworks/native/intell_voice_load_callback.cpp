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

#include "intell_voice_load_callback.h"
#include "intell_voice_log.h"
#include "intell_voice_manager.h"

#define LOG_TAG "IntellVoiceLoadCallback"

namespace OHOS {
namespace IntellVoice {
IntellVoiceLoadCallback::IntellVoiceLoadCallback(OnLoadSystemAbilitySuccessCb loadSASuccessCb,
    OnLoadSystemAbilityFailCb loadSAFailCb) : loadSASuccessCb_(loadSASuccessCb), loadSAFailCb_(loadSAFailCb)
{
}

void IntellVoiceLoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const sptr<IRemoteObject> &remoteObject)
{
    INTELL_VOICE_LOG_INFO("Load system ability success!");
    loadSASuccessCb_(remoteObject);
}

void IntellVoiceLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    INTELL_VOICE_LOG_INFO("Load system ability failed!");
    loadSAFailCb_();
}
}
}

