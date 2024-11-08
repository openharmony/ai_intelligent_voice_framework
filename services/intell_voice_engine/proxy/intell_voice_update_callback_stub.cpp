/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "intell_voice_update_callback_stub.h"
#include "intell_voice_log.h"

#define LOG_TAG "IntellVoiceUpdateCallbackStub"

using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;

namespace OHOS {
namespace IntellVoice {
int IntellVoiceUpdateCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    INTELL_VOICE_LOG_INFO("OnRemoteRequest update");
    if (data.ReadInterfaceToken() != IIntelligentVoiceUpdateCallback::GetDescriptor()) {
        INTELL_VOICE_LOG_ERROR("ReadInterfaceToken failed");
        return -1;
    }
    IIntelligentVoiceUpdateCallback::Code msgCode = static_cast<IIntelligentVoiceUpdateCallback::Code>(code);
    switch (msgCode) {
        case ON_INTELL_VOICE_UPDATE_EVENT: {
            OnUpdateCompleteCallback(data);
            reply.WriteInt32(0);
            break;
        }
        default: {
            INTELL_VOICE_LOG_WARN("invalid msgCode:%{public}d", msgCode);
            reply.WriteInt32(-1);
            break;
        }
    }
    return 0;
}

void IntellVoiceUpdateCallbackStub::OnUpdateCompleteCallback(MessageParcel &data)
{
    OnUpdateComplete(data.ReadInt32());
}
}
}