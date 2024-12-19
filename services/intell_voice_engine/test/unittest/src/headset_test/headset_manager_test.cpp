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

#include "headset_manager_test.h"
#include "adapter_host_manager_test.h"
#include "intell_voice_log.h"

#define LOG_TAG "HeadsetManagerTest"

using namespace std;
using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {

HeadsetManagerTest::HeadsetManagerTest() {}

HeadsetManagerTest::~HeadsetManagerTest() {}

int32_t HeadsetManagerTest::InitRecognize()
{
    std::lock_guard<std::mutex> lock(headsetMutex_);
    if (headsetImpl_ != nullptr) {
        INTELL_VOICE_LOG_WARN("headsetImpl already exist");
        return 0;
    }

    headsetImpl_ = UniquePtrFactory<HeadsetWakeupEngineTest>::CreateInstance();
    if (headsetImpl_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to allocate headset impl");
        return -1;
    }
    headsetImpl_->InitState();

    StateMsg msg(EVENT_INIT);
    if (headsetImpl_->Handle(msg) != 0) {
        INTELL_VOICE_LOG_ERROR("init headset wakeup engine impl failed");
    }

    int16_t result = 0;
    StateMsg msg1(INIT_DONE, &result, sizeof(int32_t));
    headsetImpl_->Handle(msg1);
    return 0;
}

int32_t HeadsetManagerTest::StartRecognize()
{
    std::lock_guard<std::mutex> lock(headsetMutex_);
    if (headsetImpl_ == nullptr) {
        INTELL_VOICE_LOG_WARN("headsetImpl is nullptr");
        return -1;
    }

    StateMsg msg(START_RECOGNIZE);
    if (headsetImpl_->Handle(msg) != 0) {
        INTELL_VOICE_LOG_ERROR("start headset wakeup engine impl failed");
    }

    OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineCallBackEvent event;
    event.msgId = static_cast<OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineMessageType>(
        HDI::IntelligentVoice::Engine::V1_2::INTELL_VOICE_ENGINE_MSG_HEADSET_RECOGNIZE_COMPLETE);
    event.result = static_cast<OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineErrors>(0);
    event.info = "";
    StateMsg msg1(RECOGNIZE_COMPLETE, reinterpret_cast<void *>(&event),
                  sizeof(OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineCallBackEvent));
    headsetImpl_->Handle(msg1);
    return 0;
}
}  // namespace IntellVoiceEngine
}  // namespace OHOS
