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
#include "adapter_host_manager_test.h"

namespace OHOS {
namespace IntellVoiceEngine {
AdapterHostManagerTest::AdapterHostManagerTest() {}

AdapterHostManagerTest::~AdapterHostManagerTest() {}

bool AdapterHostManagerTest::Init(const IntellVoiceEngineAdapterDescriptor &desc)
{
    return true;
}

int32_t AdapterHostManagerTest::SetCallback(const sptr<IIntellVoiceEngineCallback> &engineCallback)
{
    return 0;
}

int32_t AdapterHostManagerTest::Attach(
    const OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineAdapterInfo &info)
{
    return 0;
}

int32_t AdapterHostManagerTest::Detach()
{
    return 0;
}

int32_t AdapterHostManagerTest::SetParameter(const std::string &keyValueList)
{
    return 0;
}

int32_t AdapterHostManagerTest::GetParameter(const std::string &keyList, std::string &valueList)
{
    return 0;
}

int32_t AdapterHostManagerTest::Start(const OHOS::HDI::IntelligentVoice::Engine::V1_0::StartInfo &info)
{
    return 0;
}

int32_t AdapterHostManagerTest::Stop()
{
    return 0;
}

int32_t AdapterHostManagerTest::WriteAudio(const std::vector<uint8_t> &buffer)
{
    return 0;
}

int32_t AdapterHostManagerTest::Read(OHOS::HDI::IntelligentVoice::Engine::V1_0::ContentType type, sptr<Ashmem> &buffer)
{
    return 0;
}

int32_t AdapterHostManagerTest::GetWakeupPcm(std::vector<uint8_t> &data)
{
    return 0;
}

int32_t AdapterHostManagerTest::Evaluate(const std::string &word, EvaluationResultInfo &info)
{
    return 0;
}
}  // namespace IntellVoiceEngine
}  // namespace OHOS
