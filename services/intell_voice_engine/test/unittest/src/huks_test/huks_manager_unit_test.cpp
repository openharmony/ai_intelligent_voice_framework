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
#include "intell_voice_util.h"
#include "huks_manager_test.h"

#define LOG_TAG "HuksManagerUnitTest"

#define private public

using namespace testing::ext;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::IntellVoiceEngine;

class HuksManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void HuksManagerUnitTest::SetUpTestCase(void) {}

void HuksManagerUnitTest::TearDownTestCase(void) {}

void HuksManagerUnitTest::SetUp(void) {}

void HuksManagerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test Template HuksManagerUnitTest
 * @tc.number: HuksManagerUnitTest_001
 * @tc.desc  : Test For Encrypt And Decrypt
 */
HWTEST_F(HuksManagerUnitTest, HuksManagerUnitTest_001, TestSize.Level1)
{
    auto huksManagerTest = std::make_shared<HuksManagerTest>();
    EXPECT_NE(huksManagerTest, nullptr);

    std::string filePath = "/data/test/resource/";
    std::shared_ptr<uint8_t> buffer = nullptr;
    uint32_t size = 0;
    IntellVoiceUtil::ReadFile(filePath, buffer, size);
    auto inData = CreateArrayBuffer<uint8_t>(size, buffer.get());

    std::unique_ptr<OHOS::IntellVoiceUtils::Uint8ArrayBuffer> outData1 = nullptr;

    bool ret1 = huksManagerTest->Encrypt(inData, outData1);
    EXPECT_EQ(ret1, true);

    std::unique_ptr<OHOS::IntellVoiceUtils::Uint8ArrayBuffer> outData2 = nullptr;

    bool ret2 = huksManagerTest->Decrypt(outData1, outData2);
    EXPECT_EQ(ret1, true);

    EXPECT_EQ(inData->GetSize(), outData2->GetSize());
    EXPECT_EQ(*(inData->GetData()), *(outData2->GetData()));
}