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
#ifndef INTELL_VOICE_ADAPTER_LISTENER_H
#define INTELL_VOICE_ADAPTER_LISTENER_H

#include "v1_0/intell_voice_engine_types.h"

namespace OHOS {
namespace IntellVoiceEngine {
class IntellVoiceAdapterListener {
public:
    virtual ~IntellVoiceAdapterListener() = default;
    virtual void OnIntellVoiceHdiEvent(
        const OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineCallBackEvent &event) = 0;
};
}  // namespace IntellVoice
}  // namespace OHOS
#endif