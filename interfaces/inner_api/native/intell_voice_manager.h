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

#ifndef INTELL_VOICE_MANAGER_H
#define INTELL_VOICE_MANAGER_H

#include <condition_variable>
#include "iremote_object.h"
#include "iremote_broker.h"
#include "i_intell_voice_engine.h"
#include "i_intell_voice_service.h"

namespace OHOS {
namespace IntellVoice {
using namespace std;
using namespace OHOS::IntellVoiceEngine;

class IntellVoiceManager {
public:
    static IntellVoiceManager *GetInstance();

    int32_t CreateIntellVoiceEngine(IntellVoiceEngineType type, sptr<IIntellVoiceEngine> &inst);
    int32_t ReleaseIntellVoiceEngine(IntellVoiceEngineType type);
    void LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);
    void LoadSystemAbilityFail();
    bool Init();

    int32_t RegisterServiceDeathRecipient(sptr<OHOS::IRemoteObject::DeathRecipient> callback);
    int32_t DeregisterServiceDeathRecipient(sptr<OHOS::IRemoteObject::DeathRecipient> callback);

private:
    IntellVoiceManager();
    ~IntellVoiceManager();

    std::mutex mutex_;
    std::condition_variable proxyConVar_;
    sptr<IIntellVoiceService> g_sProxy;
};
}  // namespace IntellVoice
}  // namespace OHOS

#endif
