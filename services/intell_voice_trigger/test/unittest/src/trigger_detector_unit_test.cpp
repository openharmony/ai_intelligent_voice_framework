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
#include "trigger_base_type.h"
#include "trigger_detector.h"
#include "trigger_detector_callback.h"

#define LOG_TAG "TriggerDetectorTest"

#define private public

using namespace testing::ext;
using namespace OHOS::IntellVoiceTrigger;

class TriggerDetectorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TriggerDetectorTest::SetUpTestCase(void)
{}

void TriggerDetectorTest::TearDownTestCase(void)
{}

void TriggerDetectorTest::SetUp(void)
{}

void TriggerDetectorTest::TearDown(void)
{}

HWTEST_F(TriggerDetectorTest, HandleCallback_001, TestSize.Level1)
{
    INTELL_VOICE_LOG_INFO("test start");
    auto cb = std::make_shared<TriggerDetectorCallback>([]() {
        INTELL_VOICE_LOG_INFO("receive detect msg, begin wait");
        sleep(1);
        INTELL_VOICE_LOG_INFO("receive detect msg, stop wait");
    });
    ASSERT_NE(cb, nullptr);
    auto detector = std::make_shared<TriggerDetector>(1, nullptr, cb);
    ASSERT_NE(detector, nullptr);
    cb = nullptr;

    auto genericEvent = std::make_shared<GenericTriggerEvent>();
    ASSERT_NE(genericEvent, nullptr);
    detector->callback_->OnGenericTriggerDetected(genericEvent);
    detector = nullptr;
    INTELL_VOICE_LOG_INFO("test end");
}