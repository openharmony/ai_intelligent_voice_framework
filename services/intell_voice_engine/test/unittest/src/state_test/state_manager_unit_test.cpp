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
#include "state_manager_test.h"

#define LOG_TAG "StateManagerUnitTest"

#define private public

using namespace testing::ext;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::IntellVoiceEngine;

class StateManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void StateManagerUnitTest::SetUpTestCase(void) {}

void StateManagerUnitTest::TearDownTestCase(void) {}

void StateManagerUnitTest::SetUp(void) {}

void StateManagerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test Template StateManagerUnitTest
 * @tc.number: StateManagerUnitTest_001
 * @tc.desc  : Test For Module States
 */
HWTEST_F(StateManagerUnitTest, StateManagerUnitTest_001, TestSize.Level1)
{
    INTELL_VOICE_LOG_INFO("StateManagerUnitTest start");
    auto stateManagerTest = new (std::nothrow) StateManagerTest();
    EXPECT_NE(stateManagerTest, nullptr);

    stateManagerTest->Init();

    stateManagerTest->Handle(StateMsg(EVENT_SECOND));
    EXPECT_EQ(SECOND, stateManagerTest->CurrState().state);

    stateManagerTest->Handle(StateMsg(EVENT_THIRD));
    EXPECT_EQ(THIRD, stateManagerTest->CurrState().state);

    stateManagerTest->Handle(StateMsg(EVENT_FOURTH));
    EXPECT_EQ(FOURTH, stateManagerTest->CurrState().state);

    stateManagerTest->Handle(StateMsg(EVENT_FIRTH));
    EXPECT_EQ(FIRST, stateManagerTest->CurrState().state);

    stateManagerTest = nullptr;
    INTELL_VOICE_LOG_INFO("StateManagerUnitTest end");
}
