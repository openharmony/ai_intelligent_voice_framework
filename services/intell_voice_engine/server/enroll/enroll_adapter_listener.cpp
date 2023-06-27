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
#include "enroll_adapter_listener.h"

#include <thread>
#include "intell_voice_log.h"

#define LOG_TAG "EnrollAdapterListener"

using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;

namespace OHOS {
namespace IntellVoiceEngine {
EnrollAdapterListener::EnrollAdapterListener(const sptr<IIntelligentVoiceEngineCallback> &cb,
    OnEnrollEventCb enrollEventCb) : cb_(cb), enrollEventCb_(enrollEventCb)
{
    INTELL_VOICE_LOG_INFO("constructor");
}

EnrollAdapterListener::~EnrollAdapterListener()
{
    INTELL_VOICE_LOG_INFO("destructor");
}

void EnrollAdapterListener::OnIntellVoiceHdiEvent(const IntellVoiceEngineCallBackEvent& event)
{
    INTELL_VOICE_LOG_INFO("OnIntellVoiceHdiEvent");

    enrollEventCb_(event.msgId, event.result);

    if (cb_ != nullptr) {
        cb_->OnIntellVoiceEngineEvent(event);
    }
}
}
}