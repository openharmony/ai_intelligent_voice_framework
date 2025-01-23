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
#include <gtest/gtest.h>
#include <unistd.h>
#include <fstream>
#include <iostream>

#include <cstdio>
#include "intell_voice_log.h"
#define private public
#include "trigger_manager.h"
#include "trigger_detector.h"
#include "trigger_base_type.h"
#include "trigger_detector_callback.h"

#define LOG_TAG "TriggerManagerTest"

using namespace OHOS::IntellVoiceTrigger;
using namespace OHOS;
using namespace testing::ext;
using namespace std;

static std::shared_ptr<TriggerManager> triggerManager = nullptr;

class TriggerManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void TriggerManagerTestCallBack();
    void ReadFile(const std::string &path);
    std::vector<uint8_t> modelData;
    bool testResult = false;
};

void TriggerManagerTest::SetUpTestCase(void)
{}

void TriggerManagerTest::TearDownTestCase(void)
{}

void TriggerManagerTest::SetUp(void)
{
    triggerManager = std::shared_ptr<TriggerManager>(new (std::nothrow) TriggerManager());
    ASSERT_NE(triggerManager, nullptr);
    ReadFile("/data/test/resource/model_one.txt");
}

void TriggerManagerTest::TearDown(void)
{
    triggerManager = nullptr;
}

void TriggerManagerTest::TriggerManagerTestCallBack()
{
    INTELL_VOICE_LOG_INFO("enter");
    testResult = true;
}

void TriggerManagerTest::ReadFile(const std::string &path)
{
    INTELL_VOICE_LOG_INFO("path: %{public}s", path.c_str());
    ifstream infile;
    infile.open(path, ios::in);

    if (!infile.is_open()) {
        INTELL_VOICE_LOG_INFO("open file failed");
    }

    infile.seekg(0, infile.end);
    uint32_t modelSize = static_cast<uint32_t>(infile.tellg());
    std::vector<uint8_t> datas(modelSize);
    infile.seekg(0, infile.beg);
    infile.read(reinterpret_cast<char *>(datas.data()), modelSize);
    infile.close();

    modelData = datas;
}

HWTEST_F(TriggerManagerTest, start_recognition_001, TestSize.Level1)
{
    int32_t uuid = 11;
    triggerManager->UpdateModel(modelData, uuid, TriggerModelType::VOICE_WAKEUP_TYPE);
    triggerManager->CreateDetector(uuid,
        std::bind(&TriggerManagerTest::TriggerManagerTestCallBack, this));
    sleep(1);
    triggerManager->detectors_[uuid]->StartRecognition();
    triggerManager->DeleteModel(uuid);

    EXPECT_EQ(true, testResult);
}
