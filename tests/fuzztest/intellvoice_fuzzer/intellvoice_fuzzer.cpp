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
#include <cstddef>
#include <cstdint>

#include "intell_voice_manager.h"
#include "intell_voice_log.h"
#include "i_intell_voice_engine.h"
#include "i_intell_voice_engine_callback.h"
#include "enroll_intell_voice_engine.h"
#include "wakeup_intell_voice_engine.h"

const int32_t LIMITSIZE = 4;

using namespace std;
using namespace OHOS::IntellVoiceEngine;
using namespace OHOS::IntellVoice;

namespace OHOS {
class EngineEventFuzzCallback : public OHOS::IntellVoiceEngine::IIntellVoiceEngineEventCallback {
public:
    explicit EngineEventFuzzCallback() = default;
    virtual ~EngineEventFuzzCallback() = default;
    virtual void OnEvent(const OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineCallBackEvent &param) {};
};

void IntellVoiceManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < LIMITSIZE)) {
        return;
    }
    IntellVoiceEngineType type = *reinterpret_cast<const IntellVoiceEngineType *>(data);
    sptr<IIntellVoiceEngine> engine;
    IntellVoiceManager::GetInstance()->CreateIntellVoiceEngine(type, engine);
    IntellVoiceManager::GetInstance()->ReleaseIntellVoiceEngine(type);
}

void EnrollEngineFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < LIMITSIZE)) {
        return;
    }
    EnrollIntelligentVoiceEngineDescriptor descriptor = {};
    auto enrollEngine = std::make_shared<EnrollIntellVoiceEngine>(descriptor);

    EngineConfig config = {"zh_CN", "zh_CN"};
    enrollEngine->Init(config);

    bool isLast = false;
    enrollEngine->Start(isLast);

    enrollEngine->Commit();

    int32_t sensibility = *reinterpret_cast<const int32_t *>(data);
    enrollEngine->SetSensibility(sensibility);

    WakeupHapInfo info = {"com.aibase", "WakeUpExtAbility"};
    enrollEngine->SetWakeupHapInfo(info);

    enrollEngine->SetParameter("key", "value");

    shared_ptr<EngineEventFuzzCallback> engineEventFuzzCallback =
        std::make_shared<EngineEventFuzzCallback>();
    enrollEngine->SetCallback(engineEventFuzzCallback);

    enrollEngine->Stop();

    enrollEngine->Release();
}

void WakeupEngineFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < LIMITSIZE)) {
        return;
    }

    WakeupIntelligentVoiceEngineDescriptor descriptor = {};
    auto wakeEngine = std::make_shared<WakeupIntellVoiceEngine>(descriptor);

    int32_t sensibility = *reinterpret_cast<const int32_t *>(data);
    wakeEngine->SetSensibility(sensibility);

    WakeupHapInfo info = {"com.aibase", "WakeUpExtAbility"};
    wakeEngine->SetWakeupHapInfo(info);

    wakeEngine->SetParameter("key", "value");

    shared_ptr<EngineEventFuzzCallback> engineEventFuzzCallback =
        std::make_shared<EngineEventFuzzCallback>();
    wakeEngine->SetCallback(engineEventFuzzCallback);

    wakeEngine->Release();
}
} // namespace.OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::IntellVoiceManagerFuzzTest(data, size);
    OHOS::EnrollEngineFuzzTest(data, size);
    OHOS::WakeupEngineFuzzTest(data, size);
    return 0;
}
