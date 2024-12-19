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

#include <gtest/gtest.h>
#include <unistd.h>
#include <cstdio>

#include "intell_voice_log.h"
#include "headset_manager_test.h"

#define LOG_TAG "HeadsetManagerUnitTest"

#define private public

using namespace testing::ext;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::IntellVoiceEngine;

class HeadsetManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void HeadsetManagerUnitTest::SetUpTestCase(void) {}

void HeadsetManagerUnitTest::TearDownTestCase(void) {}

void HeadsetManagerUnitTest::SetUp(void) {}

void HeadsetManagerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test Template HeadsetManagerUnitTest
 * @tc.number: HeadsetManagerUnitTest_001
 * @tc.desc  : Test For HeadsetWakeupEngine
 */
HWTEST_F(HeadsetManagerUnitTest, HeadsetManagerUnitTest_001, TestSize.Level1)
{
    INTELL_VOICE_LOG_INFO("HeadsetManagerUnitTest start");
    auto headsetManagerTest = std::make_shared<HeadsetManagerTest>();
    headsetManagerTest->InitRecognize();
    sleep(1);
    EXPECT_EQ(OHOS::IntellVoiceEngine::HeadsetWakeupEngineImpl::INITIALIZED,
              headsetManagerTest->headsetImpl_->CurrState().state);
    headsetManagerTest->StartRecognize();
    sleep(1);
    EXPECT_EQ(OHOS::IntellVoiceEngine::HeadsetWakeupEngineImpl::INITIALIZED,
              headsetManagerTest->headsetImpl_->CurrState().state);
    headsetManagerTest->StartRecognize();
    sleep(1);
    EXPECT_EQ(OHOS::IntellVoiceEngine::HeadsetWakeupEngineImpl::INITIALIZED,
              headsetManagerTest->headsetImpl_->CurrState().state);
    INTELL_VOICE_LOG_INFO("HeadsetManagerUnitTest end");
}