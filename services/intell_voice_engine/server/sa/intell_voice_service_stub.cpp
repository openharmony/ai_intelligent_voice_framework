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

#include "intell_voice_service_stub.h"
#include "intell_voice_log.h"

#define LOG_TAG "IntellVoiceServiceStub"

namespace OHOS {
namespace IntellVoiceEngine {
int32_t IntellVoiceServiceStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IIntellVoiceService::GetDescriptor()) {
        INTELL_VOICE_LOG_ERROR("token mismatch");
        return -1;
    }

    IntellVoiceEngineType type = static_cast<IntellVoiceEngineType>(data.ReadInt32());
    int32_t ret = 0;

    sptr<IIntellVoiceEngine> engine;
    switch (code) {
        case HDI_INTELL_VOICE_SERVICE_CREATE_ENGINE:
            ret = CreateIntellVoiceEngine(type, engine);
            reply.WriteRemoteObject(engine->AsObject());
            return ret;

        case HDI_INTELL_VOICE_SERVICE_RELEASE_ENGINE:
            return ReleaseIntellVoiceEngine(type);
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}
}
}
