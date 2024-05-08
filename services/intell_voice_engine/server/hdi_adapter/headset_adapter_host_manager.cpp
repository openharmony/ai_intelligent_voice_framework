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
#include "headset_adapter_host_manager.h"
#include "headset_host_manager.h"

#include "intell_voice_log.h"
#include "intell_voice_util.h"

#define LOG_TAG "HeadsetAdapterHostMgr"

using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {
HeadsetAdapterHostManager::~HeadsetAdapterHostManager()
{
    headsetAdapterProxy1_0_ = nullptr;
}

bool HeadsetAdapterHostManager::Init(const IntellVoiceEngineAdapterDescriptor &desc)
{
    INTELL_VOICE_LOG_INFO("enter");
    const auto &headsetHostProxy1_0 = HeadsetHostManager::GetInstance().GetHeadsetHostProxy1_0();
    CHECK_CONDITION_RETURN_FALSE(headsetHostProxy1_0 == nullptr, "headset host proxy_v1_0 is null");

    headsetHostProxy1_0->CreateAdapter(desc, headsetAdapterProxy1_0_);
    if (headsetAdapterProxy1_0_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("engine adapter proxy1_0_ is nullptr");
        return false;
    }
    return true;
}

int32_t HeadsetAdapterHostManager::SetCallback(const sptr<IIntellVoiceEngineCallback> &engineCallback)
{
    CHECK_CONDITION_RETURN_RET(headsetAdapterProxy1_0_ == nullptr, -1, "adapter proxy is null");
    return headsetAdapterProxy1_0_->SetCallback(engineCallback);
}

int32_t HeadsetAdapterHostManager::Attach(
    const OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineAdapterInfo &info)
{
    CHECK_CONDITION_RETURN_RET(headsetAdapterProxy1_0_ == nullptr, -1, "adapter proxy is null");
    return headsetAdapterProxy1_0_->Attach(info);
}

int32_t HeadsetAdapterHostManager::Detach()
{
    CHECK_CONDITION_RETURN_RET(headsetAdapterProxy1_0_ == nullptr, -1, "adapter proxy is null");
    return headsetAdapterProxy1_0_->Detach();
}

int32_t HeadsetAdapterHostManager::SetParameter(const std::string &keyValueList)
{
    CHECK_CONDITION_RETURN_RET(headsetAdapterProxy1_0_ == nullptr, -1, "adapter proxy is null");
    return headsetAdapterProxy1_0_->SetParameter(keyValueList);
}

int32_t HeadsetAdapterHostManager::GetParameter(const std::string &keyList, std::string &valueList)
{
    CHECK_CONDITION_RETURN_RET(headsetAdapterProxy1_0_ == nullptr, -1, "adapter proxy is null");
    return headsetAdapterProxy1_0_->GetParameter(keyList, valueList);
}

int32_t HeadsetAdapterHostManager::Start(const OHOS::HDI::IntelligentVoice::Engine::V1_0::StartInfo &info)
{
    CHECK_CONDITION_RETURN_RET(headsetAdapterProxy1_0_ == nullptr, -1, "adapter proxy is null");
    return headsetAdapterProxy1_0_->Start(info);
}

int32_t HeadsetAdapterHostManager::Stop()
{
    CHECK_CONDITION_RETURN_RET(headsetAdapterProxy1_0_ == nullptr, -1, "adapter proxy is null");
    return headsetAdapterProxy1_0_->Stop();
}

int32_t HeadsetAdapterHostManager::WriteAudio(const std::vector<uint8_t> &buffer)
{
    CHECK_CONDITION_RETURN_RET(headsetAdapterProxy1_0_ == nullptr, -1, "adapter proxy is null");
    return headsetAdapterProxy1_0_->WriteAudio(buffer);
}

int32_t HeadsetAdapterHostManager::Read(OHOS::HDI::IntelligentVoice::Engine::V1_0::ContentType /* type */,
    sptr<Ashmem> & /* buffer */)
{
    return 0;
}

int32_t HeadsetAdapterHostManager::GetWakeupPcm(std::vector<uint8_t> & /* data */)
{
    return 0;
}

int32_t HeadsetAdapterHostManager::Evaluate(const std::string & /* word */, EvaluationResultInfo & /* info */)
{
    return 0;
}
}
}
