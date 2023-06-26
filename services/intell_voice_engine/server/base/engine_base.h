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
#ifndef ENGINE_BASE_H
#define ENGINE_BASE_H
#include <memory>
#include <mutex>
#include <string>
#include <map>
#include "intell_voice_engine_stub.h"
#include "v1_0/iintell_voice_engine_adapter.h"

namespace OHOS {
namespace IntellVoiceEngine {
enum EngineRelationType {
    CONCURRENCY_TYPE,
    PREEMPTION_TYPE,
    REPLACEMENT_TYPE,
};

class EngineBase : public IntellVoiceEngineStub {
public:
    ~EngineBase() = default;
    virtual bool Init() = 0;
    int32_t SetParameter(const std::string &keyValueList) override;
    std::string GetParameter(const std::string &key) override;
    int32_t WriteAudio(const uint8_t *buffer, uint32_t size) override;
    int32_t Stop() override;
    virtual void OnDetected() {};
    bool IsRunning()
    {
        return isRunning_;
    }
    EngineRelationType GetEngineRelationType()
    {
        return type_;
    }
    int32_t GetPriority()
    {
        return priority_;
    }

protected:
    EngineBase();
    void SplitStringToKVPair(const std::string &inputStr, std::map<std::string, std::string> &kvpairs);
    std::mutex mutex_;
    sptr<OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineAdapter> adapter_ = nullptr;
    OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineAdapterDescriptor desc_;

    bool isRunning_ = false;
    EngineRelationType type_ = CONCURRENCY_TYPE;
    int32_t priority_ = 0;
};
}
}
#endif