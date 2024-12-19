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
#ifndef ADAPTER_MANAGER_HOST_TEST_H
#define ADAPTER_MANAGER_HOST_TEST_H

#include "i_adapter_host_manager.h"

namespace OHOS {
namespace IntellVoiceEngine {
class AdapterHostManagerTest : public IAdapterHostManager {
public:
    AdapterHostManagerTest();
    ~AdapterHostManagerTest() override;
    bool Init(const IntellVoiceEngineAdapterDescriptor &desc) override;
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
};
}
}
#endif
