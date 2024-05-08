/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef I_ADAPTER_HOST_MANAGER_H
#define I_ADAPTER_HOST_MANAGER_H

#include "v1_0/iintell_voice_engine_manager.h"
#include "v1_2/iintell_voice_engine_manager.h"
#include "v1_0/iintell_voice_engine_adapter.h"
#include "v1_2/iintell_voice_engine_adapter.h"

namespace OHOS {
namespace IntellVoiceEngine {
using OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineAdapterDescriptor;
using OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineCallback;
using OHOS::HDI::IntelligentVoice::Engine::V1_2::EvaluationResultInfo;

class IAdapterHostManager {
public:
    IAdapterHostManager() = default;
    virtual ~IAdapterHostManager() = default;
    virtual bool Init(const IntellVoiceEngineAdapterDescriptor &desc) = 0;
    virtual int32_t SetCallback(const sptr<IIntellVoiceEngineCallback> &engineCallback) = 0;
    virtual int32_t Attach(const OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineAdapterInfo &info) = 0;
    virtual int32_t Detach() = 0;
    virtual int32_t SetParameter(const std::string &keyValueList) = 0;
    virtual int32_t GetParameter(const std::string &keyList, std::string &valueList) = 0;
    virtual int32_t Start(const OHOS::HDI::IntelligentVoice::Engine::V1_0::StartInfo &info) = 0;
    virtual int32_t Stop() = 0;
    virtual int32_t WriteAudio(const std::vector<uint8_t> &buffer) = 0;
    virtual int32_t Read(OHOS::HDI::IntelligentVoice::Engine::V1_0::ContentType type, sptr<Ashmem> &buffer) = 0;
    virtual int32_t GetWakeupPcm(std::vector<uint8_t> &data) = 0;
    virtual int32_t Evaluate(const std::string &word, EvaluationResultInfo &info) = 0;
};
}
}
#endif