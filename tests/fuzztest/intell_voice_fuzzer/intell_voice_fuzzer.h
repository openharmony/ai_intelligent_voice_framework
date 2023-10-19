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
#ifndef INTELL_VOICE_FUZZER_H
#define INTELL_VOICE_FUZZER_H

#include "i_intell_voice_engine_callback.h"
#include "i_intell_voice_engine.h"

#define FUZZ_PROJECT_NAME "intell_voice_fuzzer"

namespace OHOS {
class EngineEventFuzzCallback : public OHOS::IntellVoiceEngine::IIntellVoiceEngineEventCallback {
public:
    explicit EngineEventFuzzCallback() = default;
    virtual ~EngineEventFuzzCallback() = default;
    virtual void OnEvent(const OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineCallBackEvent &param) {};
};
}
#endif // INTELL_VOICE_FUZZER_H_