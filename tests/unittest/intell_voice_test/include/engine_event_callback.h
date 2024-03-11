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

#ifndef ENGINE_EVENT_CALLBACK_H
#define ENGINE_EVENT_CALLBACK_H

#include "i_intell_voice_engine_callback.h"
#include "i_intell_voice_engine.h"
#include "wait_for_result.h"

namespace OHOS {
namespace IntellVoiceTests {
class EngineEventCallback : public OHOS::IntellVoiceEngine::IIntellVoiceEngineEventCallback {
public:
    EngineEventCallback(sptr<OHOS::IntellVoiceEngine::IIntellVoiceEngine> &engine, WaitForResult *wait)
    {
        engine_ = engine;
        waitForResult_ = wait;
    }
    void OnEvent(const OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineCallBackEvent &param) override;

private:
    void ReadFile(const std::string &path);

private:
    int32_t startCnt_ = 0;
    uint32_t pcmSize_ = 0;
    sptr<OHOS::IntellVoiceEngine::IIntellVoiceEngine> engine_ = nullptr;
    std::shared_ptr<uint8_t> pcmData_ = nullptr;
    WaitForResult *waitForResult_;
};
}
}
#endif