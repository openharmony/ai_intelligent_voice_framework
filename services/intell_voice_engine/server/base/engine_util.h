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
#ifndef ENGINE_UTIL_H
#define ENGINE_UTIL_H
#include <memory>
#include <mutex>
#include <string>
#include <map>
#include <ashmem.h>
#include "adapter_host_manager.h"

namespace OHOS {
namespace IntellVoiceEngine {
class EngineUtil {
public:
    EngineUtil();
    ~EngineUtil() = default;
    bool CreateAdapterInner(OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineAdapterType type);
    int32_t SetParameter(const std::string &keyValueList);
    std::string GetParameter(const std::string &key);
    int32_t WriteAudio(const uint8_t *buffer, uint32_t size);
    int32_t Stop();
    int32_t GetWakeupPcm(std::vector<uint8_t> &data);
    int32_t Evaluate(const std::string &word, EvaluationResultInfo &info);
    bool SetDspFeatures();
    void ProcDspModel();
    void ReleaseAdapterInner();

protected:
    static void SplitStringToKVPair(const std::string &inputStr, std::map<std::string, std::string> &kvpairs);
    std::shared_ptr<AdapterHostManager> adapter_ = nullptr;
    OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineAdapterDescriptor desc_;

private:
    static void WriteBufferFromAshmem(uint8_t *&buffer, uint32_t size, sptr<OHOS::Ashmem> ashmem);
};
}
}
#endif