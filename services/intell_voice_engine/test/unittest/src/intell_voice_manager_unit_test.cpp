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

#include "array_buffer_util.h"
#include "intell_voice_log.h"
#include "intell_voice_util.h"
#include "intell_voice_manager_test.h"

#define LOG_TAG "IntellVoiceManagerUnitTest"

#define private public

using namespace testing::ext;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::IntellVoiceEngine;

static constexpr uint32_t MAX_TASK_NUM = 5;
class IntellVoiceManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void IntellVoiceManagerUnitTest::SetUpTestCase(void)
{}

void IntellVoiceManagerUnitTest::TearDownTestCase(void)
{}

void IntellVoiceManagerUnitTest::SetUp(void)
{}

void IntellVoiceManagerUnitTest::TearDown(void)
{}

/**
 * @tc.name  : Test Template IntellVoiceManagerUnitTest
 * @tc.number: IntellVoiceManagerUnitTest_001
 * @tc.desc  : Test For Encrypt And Decrypt
 */
HWTEST_F(IntellVoiceManagerUnitTest, IntellVoiceManagerUnitTest_001, TestSize.Level1)
{
    auto intellVoiceManagerTest = std::make_shared<IntellVoiceManagerTest>();
    EXPECT_NE(intellVoiceManagerTest, nullptr);

    std::string filePath = "/data/test/resource/";
    std::shared_ptr<uint8_t> buffer = nullptr;
    uint32_t size = 0;
    IntellVoiceUtil::ReadFile(filePath, buffer, size);
    auto inData = CreateArrayBuffer<uint8_t>(size, buffer.get());

    std::unique_ptr<OHOS::IntellVoiceUtils::Uint8ArrayBuffer> outData1 = nullptr;

    bool ret1 = intellVoiceManagerTest->Encrypt(inData, outData1);
    EXPECT_EQ(ret1, true);

    std::unique_ptr<OHOS::IntellVoiceUtils::Uint8ArrayBuffer> outData2 = nullptr;

    bool ret2 = intellVoiceManagerTest->Decrypt(outData1, outData2);
    EXPECT_EQ(ret1, true);

    EXPECT_EQ(inData->GetSize(), outData2->GetSize());
    EXPECT_EQ(*(inData->GetData()), *(outData2->GetData()));
}

/**
 * @tc.name  : Test Template IntellVoiceManagerUnitTest
 * @tc.number: IntellVoiceManagerUnitTest_001
 * @tc.desc  : Test For AddAsyncTask And AddSyncTask
 */
HWTEST_F(IntellVoiceManagerUnitTest, IntellVoiceManagerUnitTest_002, TestSize.Level1)
{
    INTELL_VOICE_LOG_ERROR("IntellVoiceManagerUnitTest start");

    auto intellVoiceManagerTest = std::make_shared<IntellVoiceManagerTest>();
    EXPECT_NE(intellVoiceManagerTest, nullptr);

    int32_t ret1 = 1;
    intellVoiceManagerTest->AddAsyncTask(ret1);
    EXPECT_NE(ret1, intellVoiceManagerTest->threadId_);
    sleep(2);
    EXPECT_EQ(ret1, intellVoiceManagerTest->threadId_);

    int ret2 = 2;
    intellVoiceManagerTest->AddSyncTask(ret2);
    EXPECT_EQ(ret2, intellVoiceManagerTest->threadId_);

    INTELL_VOICE_LOG_ERROR("IntellVoiceManagerUnitTest end");
}

/**
 * @tc.name  : Test Template IntellVoiceManagerUnitTest
 * @tc.number: IntellVoiceManagerUnitTest_001
 * @tc.desc  : Test For Task Number
 */
HWTEST_F(IntellVoiceManagerUnitTest, IntellVoiceManagerUnitTest_003, TestSize.Level1)
{
    INTELL_VOICE_LOG_ERROR("IntellVoiceManagerUnitTest start");

    auto intellVoiceManagerTest = std::make_shared<IntellVoiceManagerTest>();
    EXPECT_NE(intellVoiceManagerTest, nullptr);

    int ret = 0;
    for (int i = 0; i <= MAX_TASK_NUM; i++) {
        ret++;
        intellVoiceManagerTest->AddAsyncTask(ret);
    }
    sleep(2);
    EXPECT_NE(ret, intellVoiceManagerTest->threadId_);
    INTELL_VOICE_LOG_ERROR("IntellVoiceManagerUnitTest end");
}