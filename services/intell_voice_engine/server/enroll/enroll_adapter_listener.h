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
#ifndef ENROLL_ADAPTER_LISTENER_H
#define ENROLL_ADAPTER_LISTENER_H

#include <functional>
#include "intell_voice_adapter_listener.h"
#include "i_intell_voice_engine_callback.h"

namespace OHOS {
namespace IntellVoiceEngine {
using OnEnrollEventCb = std::function<void(int32_t, int32_t)>;

class EnrollAdapterListener : public IntellVoiceAdapterListener {
public:
    EnrollAdapterListener(const sptr<IIntelligentVoiceEngineCallback> &cb, OnEnrollEventCb enrollEventCb);
    ~EnrollAdapterListener();

    void OnIntellVoiceHdiEvent(
        const OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineCallBackEvent& event) override;

private:
    sptr<IIntelligentVoiceEngineCallback> cb_ = nullptr;
    OnEnrollEventCb enrollEventCb_;
};
}
}
#endif