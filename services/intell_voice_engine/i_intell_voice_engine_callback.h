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

#ifndef I_INTELL_VOICE_ENGINE_CALLBACK_H
#define I_INTELL_VOICE_ENGINE_CALLBACK_H

#include "iremote_broker.h"
#include "v1_0/intell_voice_engine_types.h"

namespace OHOS {
namespace IntellVoiceEngine {
using OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineCallBackEvent;

class IIntellVoiceEngineEventCallback {
public:
    virtual ~IIntellVoiceEngineEventCallback() = default;
    virtual void OnEvent(const IntellVoiceEngineCallBackEvent &param) = 0;
};

class IIntelligentVoiceEngineCallback : public IRemoteBroker {
public:
    virtual void OnIntellVoiceEngineEvent(const IntellVoiceEngineCallBackEvent &event) = 0;
    enum Code {
        ON_INTELL_VOICE_ENGINE_EVENT = 0,
    };

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IIntelligentVoiceEngineCallback");
};
}  // namespace IntellVoice
}  // namespace OHOS
#endif  // I_INTELL_VOICE_ENGINE_CALLBACK_H
