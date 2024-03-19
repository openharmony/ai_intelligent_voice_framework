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
#include <cstdio>

#include "intell_voice_log.h"
#include "trigger_manager.h"
#include "trigger_base_type.h"
#include "trigger_detector_callback.h"

#define LOG_TAG "TriggerTest"

using namespace OHOS::IntellVoiceTrigger;
using namespace OHOS;
using namespace testing::ext;

static std::shared_ptr<TriggerManager> triggerManager = nullptr;

class TriggerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TriggerTest::SetUpTestCase(void)
{}

void TriggerTest::TearDownTestCase(void)
{}

void TriggerTest::SetUp(void)
{
    triggerManager = TriggerManager::GetInstance();
    ASSERT_NE(triggerManager, nullptr);
}

void TriggerTest::TearDown(void)
{
    triggerManager = nullptr;
}

HWTEST_F(TriggerTest, trigger_db_helper_001, TestSize.Level1)
{
    // insert model
    int32_t uuid = 11;
    auto model = std::make_shared<GenericTriggerModel>(uuid, 100, TriggerModel::TriggerModelType::VOICE_WAKEUP_TYPE);
    uint8_t data[4] = {0, 1, 2, 3};
    model->SetData(data, sizeof(data));
    std::vector<uint8_t> expect(data, data + (sizeof(data) / sizeof(uint8_t)));

    triggerManager->UpdateModel(model);
    auto result = triggerManager->GetModel(uuid);
    EXPECT_EQ(expect, result->GetData());

    // update model
    uint8_t newdata[4] = {0, 1, 2, 5};
    model->SetData(newdata, sizeof(newdata));

    std::vector<uint8_t>().swap(expect);
    expect.insert(expect.begin(), newdata, newdata + (sizeof(newdata) / sizeof(uint8_t)));

    triggerManager->UpdateModel(model);
    result = triggerManager->GetModel(uuid);
    EXPECT_EQ(expect, result->GetData());

    // delete model
    triggerManager->DeleteModel(uuid);
    result = triggerManager->GetModel(uuid);
    EXPECT_EQ(nullptr, result);
}
