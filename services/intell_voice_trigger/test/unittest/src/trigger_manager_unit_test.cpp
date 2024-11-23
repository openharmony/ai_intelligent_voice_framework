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
#include <fstream>
#include <iostream>

#include "intell_voice_log.h"
#include "trigger_manager_test.h"

#define LOG_TAG "TriggerDetectorTest"

#define private public

using namespace testing::ext;
using namespace OHOS::IntellVoiceTrigger;

class TriggerManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TriggerManagerUnitTest::SetUpTestCase(void)
{}

void TriggerManagerUnitTest::TearDownTestCase(void)
{}

void TriggerManagerUnitTest::SetUp(void)
{}

void TriggerManagerUnitTest::TearDown(void)
{}

/**
 * @tc.name  : Test Template IntellVoiceManagerUnitTest
 * @tc.number: IntellVoiceManagerUnitTest_001
 * @tc.desc  : Test For StartRecognition
 */
HWTEST_F(TriggerManagerUnitTest, StartRecognition_001, TestSize.Level1)
{
    auto triggerManagerTest = std::make_shared<TriggerManagerTest>();
    EXPECT_NE(triggerManagerTest, nullptr);

    int32_t uuid = 11;
    bool ret1 = triggerManagerTest->InitRecognition(uuid);
    EXPECT_EQ(ret1, true);

    bool ret2 = triggerManagerTest->StartRecognition();
    EXPECT_EQ(ret2, true);

    auto state1 = triggerManagerTest->GetState(uuid);
    EXPECT_EQ(state1, MODEL_STARTED);

    bool ret3 = triggerManagerTest->StopRecognition();
    EXPECT_EQ(ret3, true);

    auto state2 = triggerManagerTest->GetState(uuid);
    EXPECT_EQ(state2, MODEL_LOADED);

    bool ret4 = triggerManagerTest->UnloadTriggerModel();
    EXPECT_EQ(ret4, true);

    bool ret5 = triggerManagerTest->DeleteModel(uuid);
    EXPECT_EQ(ret5, true);
}

/**
 * @tc.name  : Test Template IntellVoiceManagerUnitTest
 * @tc.number: IntellVoiceManagerUnitTest_001
 * @tc.desc  : Test For OnCapturerStateChange
 */
HWTEST_F(TriggerManagerUnitTest, StartRecognition_002, TestSize.Level1)
{
    auto triggerManagerTest = std::make_shared<TriggerManagerTest>();
    EXPECT_NE(triggerManagerTest, nullptr);

    int32_t uuid = 11;
    bool ret1 = triggerManagerTest->InitRecognition(uuid);
    EXPECT_EQ(ret1, true);

    bool ret2 = triggerManagerTest->StartRecognition();
    EXPECT_EQ(ret2, true);

    auto state1 = triggerManagerTest->GetState(uuid);
    EXPECT_EQ(state1, MODEL_STARTED);

    bool ret3 = triggerManagerTest->OnCapturerStateChange(true);
    EXPECT_EQ(ret3, true);

    auto state2 = triggerManagerTest->GetState(uuid);
    EXPECT_EQ(state2, MODEL_LOADED);

    bool ret4 = triggerManagerTest->OnCapturerStateChange(false);
    EXPECT_EQ(ret4, true);

    auto state3 = triggerManagerTest->GetState(uuid);
    EXPECT_EQ(state3, MODEL_STARTED);

    bool ret5 = triggerManagerTest->StopRecognition();
    EXPECT_EQ(ret5, true);

    auto state4 = triggerManagerTest->GetState(uuid);
    EXPECT_EQ(state4, MODEL_LOADED);

    bool ret6 = triggerManagerTest->UnloadTriggerModel();
    EXPECT_EQ(ret6, true);

    bool ret7 = triggerManagerTest->DeleteModel(uuid);
    EXPECT_EQ(ret7, true);
}

/**
 * @tc.name  : Test Template IntellVoiceManagerUnitTest
 * @tc.number: IntellVoiceManagerUnitTest_001
 * @tc.desc  : Test For OnCallStateUpdated
 */
HWTEST_F(TriggerManagerUnitTest, StartRecognition_003, TestSize.Level1)
{
    auto triggerManagerTest = std::make_shared<TriggerManagerTest>();
    EXPECT_NE(triggerManagerTest, nullptr);

    int32_t uuid = 11;
    bool ret1 = triggerManagerTest->InitRecognition(uuid);
    EXPECT_EQ(ret1, true);

    bool ret2 = triggerManagerTest->StartRecognition();
    EXPECT_EQ(ret2, true);

    auto state1 = triggerManagerTest->GetState(uuid);
    EXPECT_EQ(state1, MODEL_STARTED);

    bool ret3 = triggerManagerTest->OnCallStateUpdated(1);
    EXPECT_EQ(ret3, true);

    auto state2 = triggerManagerTest->GetState(uuid);
    EXPECT_EQ(state2, MODEL_LOADED);

    bool ret4 = triggerManagerTest->OnCallStateUpdated(6);
    EXPECT_EQ(ret4, true);

    auto state3 = triggerManagerTest->GetState(uuid);
    EXPECT_EQ(state3, MODEL_STARTED);

    bool ret5 = triggerManagerTest->StopRecognition();
    EXPECT_EQ(ret5, true);

    auto state4 = triggerManagerTest->GetState(uuid);
    EXPECT_EQ(state4, MODEL_LOADED);

    bool ret6 = triggerManagerTest->UnloadTriggerModel();
    EXPECT_EQ(ret6, true);

    bool ret7 = triggerManagerTest->DeleteModel(uuid);
    EXPECT_EQ(ret7, true);
}