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
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <gtest/gtest.h>

#include "intell_voice_log.h"
#include "engine_event_callback.h"

#define LOG_TAG "EngineEventCallback"

using namespace std;

namespace {
constexpr int32_t ENROLL_CNT = 3;
static constexpr uint32_t BUFFER_SIZE = 1280;
static constexpr uint32_t WAIT_TIME = 1000;
}

namespace OHOS {
namespace IntellVoiceTests {
const std::string TEST_RESOURCE_PATH = "/data/test/resource/";

void EngineEventCallback::OnEvent(
    const OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineCallBackEvent &event)
{
    INTELL_VOICE_LOG_INFO("OnEvent EngineCallBackInfo: msgId: %{public}d, errCode: %{public}d, context: %{public}s",
        event.msgId,
        event.result,
        event.info.c_str());

    if (event.msgId == HDI::IntelligentVoice::Engine::V1_0::INTELL_VOICE_ENGINE_MSG_INIT_DONE) {
        EXPECT_EQ(event.result, HDI::IntelligentVoice::Engine::V1_0::INTELL_VOICE_ENGINE_OK);
        startCnt_ = 1;
        engine_->Start(false);
        EngineEventCallback::ReadFile(TEST_RESOURCE_PATH + "one.pcm");
    }

    if (event.msgId == HDI::IntelligentVoice::Engine::V1_0::INTELL_VOICE_ENGINE_MSG_ENROLL_COMPLETE) {
        EXPECT_EQ(event.result, HDI::IntelligentVoice::Engine::V1_0::INTELL_VOICE_ENGINE_OK);
        if (startCnt_ < ENROLL_CNT) {
            ++startCnt_;
            engine_->Start(startCnt_ == ENROLL_CNT ? true : false);
            usleep(WAIT_TIME);
            EngineEventCallback::ReadFile(TEST_RESOURCE_PATH + "one.pcm");
        } else if (startCnt_ == ENROLL_CNT) {
            engine_->SetParameter("CommitEnrollment=true");
        }
    }

    if (event.msgId == HDI::IntelligentVoice::Engine::V1_0::INTELL_VOICE_ENGINE_MSG_COMMIT_ENROLL_COMPLETE) {
        EXPECT_EQ(event.result, HDI::IntelligentVoice::Engine::V1_0::INTELL_VOICE_ENGINE_OK);
        waitForResult_->SetIsReady();
    }
}

void EngineEventCallback::ReadFile(const std::string &path)
{
    INTELL_VOICE_LOG_INFO("path: %{public}s", path.c_str());
    ifstream infile;
    infile.open(path, ios::in | ios::binary);

    if (!infile.is_open()) {
        INTELL_VOICE_LOG_INFO("open file failed");
    }

    infile.seekg(0, infile.end);
    pcmSize_ = static_cast<uint32_t>(infile.tellg());
    pcmData_ = std::shared_ptr<uint8_t>(new uint8_t[pcmSize_], [](uint8_t *p) { delete[] p; });
    if (pcmData_ == nullptr) {
        INTELL_VOICE_LOG_INFO("pcmData_ is null");
        return;
    }
    INTELL_VOICE_LOG_INFO("read pcm, pcmSize:%{public}u", pcmSize_);
    infile.seekg(0, infile.beg);
    infile.read(reinterpret_cast<char *>(pcmData_.get()), pcmSize_);
    infile.close();

    for (uint32_t i = 0; i < pcmSize_ / BUFFER_SIZE; i++) {
        engine_->WriteAudio(pcmData_.get() + i * BUFFER_SIZE, BUFFER_SIZE);
    }
    engine_->SetParameter("end_of_pcm=true");
}
}  // namespace IntellVoiceTests
}  // namespace OHOS
