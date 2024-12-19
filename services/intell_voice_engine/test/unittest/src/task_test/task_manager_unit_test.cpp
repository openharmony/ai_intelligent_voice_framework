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
#include "task_manager_test.h"

#define LOG_TAG "TaskManagerUnitTest"

#define private public

using namespace testing::ext;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::IntellVoiceEngine;

static constexpr uint32_t MAX_TASK_NUM = 5;
class TaskManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TaskManagerUnitTest::SetUpTestCase(void) {}

void TaskManagerUnitTest::TearDownTestCase(void) {}

void TaskManagerUnitTest::SetUp(void) {}

void TaskManagerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test Template TaskManagerUnitTest
 * @tc.number: TaskManagerUnitTest_001
 * @tc.desc  : Test For AddAsyncTask And AddSyncTask
 */
HWTEST_F(TaskManagerUnitTest, TaskManagerUnitTest_001, TestSize.Level1)
{
    INTELL_VOICE_LOG_INFO("TaskManagerUnitTest_001 start");

    auto taskManagerTest = std::make_shared<TaskManagerTest>();
    EXPECT_NE(taskManagerTest, nullptr);

    int32_t ret1 = 1;
    taskManagerTest->AddAsyncTask(ret1);
    EXPECT_NE(ret1, taskManagerTest->threadId_);
    sleep(2);
    EXPECT_EQ(ret1, taskManagerTest->threadId_);

    int ret2 = 2;
    taskManagerTest->AddSyncTask(ret2);
    EXPECT_EQ(ret2, taskManagerTest->threadId_);

    INTELL_VOICE_LOG_INFO("TaskManagerUnitTest_001 end");
}

/**
 * @tc.name  : Test Template TaskManagerUnitTest
 * @tc.number: TaskManagerUnitTest_002
 * @tc.desc  : Test For Task Number
 */
HWTEST_F(TaskManagerUnitTest, TaskManagerUnitTest_002, TestSize.Level1)
{
    INTELL_VOICE_LOG_INFO("TaskManagerUnitTest_002 start");

    auto taskManagerTest = std::make_shared<TaskManagerTest>();
    EXPECT_NE(taskManagerTest, nullptr);

    int ret = 0;
    for (int i = 0; i <= MAX_TASK_NUM; i++) {
        ret++;
        taskManagerTest->AddAsyncTask(ret);
    }
    sleep(2);
    EXPECT_NE(ret, taskManagerTest->threadId_);
    INTELL_VOICE_LOG_INFO("TaskManagerUnitTest_002 end");
}
