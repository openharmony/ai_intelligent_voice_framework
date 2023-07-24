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
#ifndef ADAPTER_CALLBACK_SERVICE_H
#define ADAPTER_CALLBACK_SERVICE_H

#include "v1_0/iintell_voice_engine_callback.h"
#include "intell_voice_adapter_listener.h"

namespace OHOS {
namespace IntellVoiceEngine {
using OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineCallback;
using OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineCallBackEvent;

class AdapterCallbackService final: public IIntellVoiceEngineCallback {
public:
    explicit AdapterCallbackService(std::shared_ptr<IntellVoiceAdapterListener> listener) : listener_(listener) {}
    virtual ~AdapterCallbackService() = default;

    int32_t OnIntellVoiceHdiEvent(const IntellVoiceEngineCallBackEvent& event) override;
private:
    std::shared_ptr<IntellVoiceAdapterListener> listener_ = nullptr;
};
}
}
#endif