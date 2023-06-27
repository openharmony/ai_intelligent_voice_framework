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
#ifndef ENGINE_CALLBACK_PROXY_H
#define ENGINE_CALLBACK_PROXY_H

#include "iremote_proxy.h"
#include "i_intell_voice_engine_callback.h"

namespace OHOS {
namespace IntellVoiceEngine {
using OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineCallBackEvent;

class IntellVoiceEngineCallbackProxy : public IRemoteProxy<IIntelligentVoiceEngineCallback> {
public:
    explicit IntellVoiceEngineCallbackProxy(const sptr<IRemoteObject> &impl)
        : IRemoteProxy<IIntelligentVoiceEngineCallback>(impl) {};
    virtual ~IntellVoiceEngineCallbackProxy() = default;
    void OnIntellVoiceEngineEvent(const IntellVoiceEngineCallBackEvent &event) override;
private:
    static inline BrokerDelegator<IntellVoiceEngineCallbackProxy> delegator_;
};
}
}
#endif