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
#include "engine_callback_inner.h"
#include <thread>
#include "intell_voice_log.h"

#define LOG_TAG "EngineCallbackInner"

using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;

namespace OHOS {
namespace IntellVoiceEngine {
EngineCallbackInner::EngineCallbackInner(std::shared_ptr<IIntellVoiceEngineEventCallback> cb) : cb_(cb)
{
}

void EngineCallbackInner::OnIntellVoiceEngineEvent(const IntellVoiceEngineCallBackEvent &event)
{
    INTELL_VOICE_LOG_INFO("receive event");
    if (cb_ == nullptr) {
        INTELL_VOICE_LOG_INFO("cb is null");
        return;
    }
    auto msgId = event.msgId;
    auto result = event.result;
    auto info = event.info;
    std::thread([this, msgId, result, info]() {
        IntellVoiceEngineCallBackEvent tmpEvent;
        tmpEvent.msgId = msgId;
        tmpEvent.result = result;
        tmpEvent.info = info;
        cb_->OnEvent(tmpEvent);
    }).detach();
}
}
}
