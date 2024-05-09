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
#ifndef HEADSET_MANAGER_HOST_H
#define HEADSET_MANAGER_HOST_H
#include "v1_0/iintell_voice_engine_manager.h"
#include "v1_1/iintell_voice_engine_manager.h"
#include "intell_voice_death_recipient.h"
#include "i_adapter_host_manager.h"

namespace OHOS {
namespace IntellVoiceEngine {
using OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineAdapter;
using OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineAdapterDescriptor;

class HeadsetHostManager {
public:
    HeadsetHostManager() = default;
    ~HeadsetHostManager();
    static HeadsetHostManager &GetInstance()
    {
        static HeadsetHostManager headsetHostMgr;
        return headsetHostMgr;
    }

    bool Init();
    bool RegisterEngineHDIDeathRecipient();
    void DeregisterEngineHDIDeathRecipient();
    std::shared_ptr<IAdapterHostManager> CreateEngineAdapter(const IntellVoiceEngineAdapterDescriptor &desc);
    void ReleaseEngineAdapter(const IntellVoiceEngineAdapterDescriptor &desc);
    const sptr<OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineManager> &GetHeadsetHostProxy1_0()
    {
        return headsetHostProxy1_0_;
    }

private:
    static void OnEngineHDIDiedCallback();

    sptr<OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineManager> headsetHostProxy1_0_ = nullptr;
    sptr<OHOS::IntellVoiceUtils::IntellVoiceDeathRecipient> engineHdiDeathRecipient_ = nullptr;
};
}
}
#endif
