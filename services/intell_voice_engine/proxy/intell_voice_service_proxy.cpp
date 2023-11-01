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

#include "intell_voice_service_proxy.h"
#include "intell_voice_log.h"
#include "intell_voice_death_recipient_stub.h"

#define LOG_TAG "IntellVoiceServiceProxy"
namespace OHOS {
namespace IntellVoiceEngine {
int32_t IntellVoiceServiceProxy::CreateIntellVoiceEngine(IntellVoiceEngineType type, sptr<IIntellVoiceEngine> &inst)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IIntellVoiceService::GetDescriptor());
    data.WriteInt32(static_cast<int>(type));

    if (type == INTELL_VOICE_ENROLL) {
        auto stub = new IntellVoiceDeathRecipientStub();
        data.WriteRemoteObject(stub);
    }

    Remote()->SendRequest(HDI_INTELL_VOICE_SERVICE_CREATE_ENGINE, data, reply, option);

    sptr<IRemoteObject> remote = reply.ReadRemoteObject();
    if (remote == nullptr) {
        INTELL_VOICE_LOG_ERROR("Create engine failed, engine is null");
        return -1;
    }
    inst = iface_cast<IIntellVoiceEngine>(remote);

    return 0;
}

int32_t IntellVoiceServiceProxy::ReleaseIntellVoiceEngine(IntellVoiceEngineType type)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IIntellVoiceService::GetDescriptor());
    data.WriteInt32(type);

    Remote()->SendRequest(HDI_INTELL_VOICE_SERVICE_RELEASE_ENGINE, data, reply, option);
    return reply.ReadInt32();
}
}  // namespace IntellVoiceEngine
}  // namespace OHOS
