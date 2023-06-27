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

#include "intell_voice_engine_callback_proxy.h"
#include "intell_voice_log.h"

#define LOG_TAG "IntellVoiceEngineCallbackProxy"

using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;

namespace OHOS {
namespace IntellVoiceEngine {
void IntellVoiceEngineCallbackProxy::OnIntellVoiceEngineEvent(const IntellVoiceEngineCallBackEvent &event)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    INTELL_VOICE_LOG_INFO("enter");

    if (!data.WriteInterfaceToken(IIntelligentVoiceEngineCallback::GetDescriptor())) {
        INTELL_VOICE_LOG_ERROR("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(static_cast<int32_t>(event.msgId))) {
        INTELL_VOICE_LOG_ERROR("write msgid failed");
        return;
    }

    if (!data.WriteInt32(static_cast<int32_t>(event.result))) {
        INTELL_VOICE_LOG_ERROR("write result failed");
        return;
    }

    if (!data.WriteString(event.info)) {
        INTELL_VOICE_LOG_ERROR("write info failed");
        return;
    }

    int32_t error = Remote()->SendRequest(IIntelligentVoiceEngineCallback::Code::ON_INTELL_VOICE_ENGINE_EVENT,
        data, reply, option);
    if (error != 0) {
        INTELL_VOICE_LOG_ERROR("send request error: %{public}d", error);
    }
    reply.ReadInt32();
}
}
}