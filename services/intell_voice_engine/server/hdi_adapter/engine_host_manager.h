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
#ifndef ENGINE_MANAGER_HOST_H
#define ENGINE_MANAGER_HOST_H
#include "v1_0/iintell_voice_engine_manager.h"
#include "v1_1/iintell_voice_engine_manager.h"
#include "intell_voice_death_recipient.h"

namespace OHOS {
namespace IntellVoiceEngine {
using OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineAdapter;
using OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineAdapterDescriptor;
using OHOS::HDI::IntelligentVoice::Engine::V1_1::IIntellVoiceDataOprCallback;

class EngineHostManager {
public:
    EngineHostManager() = default;
    ~EngineHostManager();
    static EngineHostManager &GetInstance()
    {
        static EngineHostManager engineHostMgr;
        return engineHostMgr;
    }

    bool Init();
    bool RegisterEngineHDIDeathRecipient();
    void DeregisterEngineHDIDeathRecipient();
    void SetDataOprCallback();
    sptr<IIntellVoiceEngineAdapter> CreateEngineAdapter(const IntellVoiceEngineAdapterDescriptor &desc);
    void ReleaseEngineAdapter(const IntellVoiceEngineAdapterDescriptor &desc);

private:
    void OnEngineHDIDiedCallback();

    sptr<OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineManager> engineHostProxy1_0_ = nullptr;
    sptr<OHOS::HDI::IntelligentVoice::Engine::V1_1::IIntellVoiceEngineManager> engineHostProxy1_1_ = nullptr;
    sptr<OHOS::IntellVoiceUtils::IntellVoiceDeathRecipient> engineHdiDeathRecipient_ = nullptr;
    sptr<IIntellVoiceDataOprCallback> dataOprCb_ = nullptr;
};
}
}
#endif
