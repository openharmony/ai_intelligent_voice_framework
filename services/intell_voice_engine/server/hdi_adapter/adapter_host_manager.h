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
#ifndef ADAPTER_MANAGER_HOST_H
#define ADAPTER_MANAGER_HOST_H
#include "v1_0/iintell_voice_engine_manager.h"
#include "v1_2/iintell_voice_engine_manager.h"
#include "v1_0/iintell_voice_engine_adapter.h"
#include "v1_2/iintell_voice_engine_adapter.h"

namespace OHOS {
namespace IntellVoiceEngine {
using OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineAdapterDescriptor;
using OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineCallback;
using OHOS::HDI::IntelligentVoice::Engine::V1_2::EvaluationResultInfo;

class AdapterHostManager : public HDI::IntelligentVoice::Engine::V1_2::IIntellVoiceEngineAdapter {
public:
    AdapterHostManager() = default;
    ~AdapterHostManager();
    bool Init(const IntellVoiceEngineAdapterDescriptor &desc,
        const sptr<OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineManager> &engineHostProxy1_0,
        const sptr<OHOS::HDI::IntelligentVoice::Engine::V1_2::IIntellVoiceEngineManager> &engineHostProxy1_2);
    int32_t SetCallback(const sptr<IIntellVoiceEngineCallback> &engineCallback) override;
    int32_t Attach(const OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineAdapterInfo &info) override;
    int32_t Detach() override;
    int32_t SetParameter(const std::string &keyValueList) override;
    int32_t GetParameter(const std::string &keyList, std::string &valueList) override;
    int32_t Start(const OHOS::HDI::IntelligentVoice::Engine::V1_0::StartInfo &info) override;
    int32_t Stop() override;
    int32_t WriteAudio(const std::vector<uint8_t> &buffer) override;
    int32_t Read(OHOS::HDI::IntelligentVoice::Engine::V1_0::ContentType type, sptr<Ashmem> &buffer) override;
    int32_t GetWakeupPcm(std::vector<uint8_t> &data) override;
    int32_t Evaluate(const std::string &word, EvaluationResultInfo &info) override;

private:
    sptr<OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineAdapter> adapterProxy1_0_ = nullptr;
    sptr<OHOS::HDI::IntelligentVoice::Engine::V1_2::IIntellVoiceEngineAdapter> adapterProxy1_2_ = nullptr;
};
}
}
#endif
