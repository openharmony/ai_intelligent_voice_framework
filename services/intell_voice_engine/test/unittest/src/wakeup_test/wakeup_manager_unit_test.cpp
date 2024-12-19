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
#include "wakeup_manager_test.h"

#define LOG_TAG "WakeupManagerUnitTest"

#define private public

using namespace testing::ext;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::IntellVoiceEngine;

class WakeupManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void WakeupManagerUnitTest::SetUpTestCase(void) {}

void WakeupManagerUnitTest::TearDownTestCase(void) {}

void WakeupManagerUnitTest::SetUp(void) {}

void WakeupManagerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test Template WakeupManagerUnitTest
 * @tc.number: WakeupManagerUnitTest_001
 * @tc.desc  : Test For WakeupEngine
 */
HWTEST_F(WakeupManagerUnitTest, WakeupManagerUnitTest_001, TestSize.Level1)
{
    INTELL_VOICE_LOG_INFO("WakeupManagerUnitTest_001 start");
    auto wakeupManagerTest = std::make_shared<WakeupManagerTest>();
    wakeupManagerTest->InitRecognize();
    EXPECT_EQ(OHOS::IntellVoiceEngine::WakeupEngineImpl::INITIALIZED,
              wakeupManagerTest->wakeupImpl_->CurrState().state);
    wakeupManagerTest->StartRecognize();
    EXPECT_EQ(OHOS::IntellVoiceEngine::WakeupEngineImpl::RECOGNIZED, wakeupManagerTest->wakeupImpl_->CurrState().state);
    INTELL_VOICE_LOG_INFO("WakeupManagerUnitTest_001 end");
}

/**
 * @tc.name  : Test Template WakeupManagerUnitTest
 * @tc.number: WakeupManagerUnitTest_002
 * @tc.desc  : Test For WakeupEngine RECOGNIZE_COMPLETE_TIMEOUT
 */
HWTEST_F(WakeupManagerUnitTest, WakeupManagerUnitTest_002, TestSize.Level1)
{
    INTELL_VOICE_LOG_INFO("WakeupManagerUnitTest_002 start");
    auto wakeupManagerTest = std::make_shared<WakeupManagerTest>();
    wakeupManagerTest->InitRecognize();
    EXPECT_EQ(OHOS::IntellVoiceEngine::WakeupEngineImpl::INITIALIZED,
              wakeupManagerTest->wakeupImpl_->CurrState().state);
    wakeupManagerTest->StartRecognize();
    sleep(3);
    EXPECT_EQ(OHOS::IntellVoiceEngine::WakeupEngineImpl::INITIALIZED,
              wakeupManagerTest->wakeupImpl_->CurrState().state);
    INTELL_VOICE_LOG_INFO("WakeupManagerUnitTest_002 end");
}

/**
 * @tc.name  : Test Template WakeupManagerUnitTest
 * @tc.number: WakeupManagerUnitTest_003
 * @tc.desc  : Test For WakeupEngine START_CAPTURER
 */
HWTEST_F(WakeupManagerUnitTest, WakeupManagerUnitTest_003, TestSize.Level1)
{
    INTELL_VOICE_LOG_INFO("WakeupManagerUnitTest_003 start");
    auto wakeupManagerTest = std::make_shared<WakeupManagerTest>();
    wakeupManagerTest->InitRecognize();
    EXPECT_EQ(OHOS::IntellVoiceEngine::WakeupEngineImpl::INITIALIZED,
              wakeupManagerTest->wakeupImpl_->CurrState().state);
    wakeupManagerTest->StartRecognize();
    EXPECT_EQ(OHOS::IntellVoiceEngine::WakeupEngineImpl::RECOGNIZED, wakeupManagerTest->wakeupImpl_->CurrState().state);
    wakeupManagerTest->StartCapturer();
    EXPECT_EQ(OHOS::IntellVoiceEngine::WakeupEngineImpl::READ_CAPTURER,
              wakeupManagerTest->wakeupImpl_->CurrState().state);
    INTELL_VOICE_LOG_INFO("WakeupManagerUnitTest_003 end");
}