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

#include "intell_voice_engine_stub.h"
#include "securec.h"
#include "intell_voice_log.h"

#define LOG_TAG "IntellVoiceEngineStub"

namespace OHOS {
namespace IntellVoiceEngine {
int32_t IntellVoiceEngineStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IIntellVoiceEngine::GetDescriptor()) {
        INTELL_VOICE_LOG_ERROR("token mismatch");
        return -1;
    }

    int32_t ret = 0;

    switch (code) {
        case INTELL_VOICE_ENGINE_SET_CALLBACK:
            object = data.ReadRemoteObject();
            SetCallback(object);
            break;
        case INTELL_VOICE_ENGINE_ATTACH:
            info.wakeupPhrase = data.ReadString();
            info.isPcmFromExternal = data.ReadBool();
            info.minBufSize = data.ReadInt32();
            info.sampleChannels = data.ReadInt32();
            info.bitsPerSample = data.ReadInt32();
            info.sampleRate = data.ReadInt32();
            ret = Attach(info);
            break;
        case INTELL_VOICE_ENGINE_DETACH:
            ret = Detach();
            break;
        case INTELL_VOICE_ENGINE_SET_PARAMETER:
            ret = SetParameter(data.ReadString());
            break;
        case INTELL_VOICE_ENGINE_GET_PARAMETER:
            str = GetParameter(data.ReadString());
            reply.WriteString(str);
            break;
        case INTELL_VOICE_ENGINE_START:
            ret = Start(data.ReadBool());
            break;
        case INTELL_VOICE_ENGINE_STOP:
            ret = Stop();
            break;
        case INTELL_VOICE_ENGINE_WRITE_AUDIO:
            size = data.ReadInt32();
            buffer = data.ReadBuffer(size);
            ret = WriteAudio(buffer, size);
            break;
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
    }

    return ret;
}
}
}