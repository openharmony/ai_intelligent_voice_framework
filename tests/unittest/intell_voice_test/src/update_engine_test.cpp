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
#include "update_engine_test.h"

#include <fstream>
#include <iostream>

#include "json/json.h"
#include "update_engine.h"
#include "intell_voice_manager.h"
#include "engine_host_manager.h"
#include "intell_voice_log.h"
#include "intell_voice_service_manager.h"
#include "silence_update_strategy.h"
#include "i_intell_voice_engine.h"
#include "history_info_mgr.h"

#define LOG_TAG "UpdateEngineTest"

#define INTELL_VOICE_BUILD_VARIANT_ROOT

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace std;
using namespace OHOS::IntellVoiceTests;
using namespace OHOS::IntellVoice;
using namespace OHOS::IntellVoiceEngine;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;

#define private public

namespace OHOS {
namespace IntellVoiceTests {
class TestWptr : public RefBase {
public:
    void Func()
    {
        wptr<TestWptr> thisWptr(this);
        std::thread([thisWptr]() {
            INTELL_VOICE_LOG_INFO("start");
            sleep(1);
            sptr<TestWptr> thisSptr = thisWptr.promote();
            if (thisSptr != nullptr) {
                INTELL_VOICE_LOG_WARN("TestWptr not null");
            } else {
                INTELL_VOICE_LOG_WARN("TestWptr is null");
            }
            INTELL_VOICE_LOG_INFO("end");
        }).detach();
    }
};

void UpdateEngineTest::SetUpTestCase(void)
{}

void UpdateEngineTest::TearDownTestCase(void)
{}

void UpdateEngineTest::SetUp(void)
{}

void UpdateEngineTest::TearDown(void)
{}

/**
 * @tc.name  : Test Template UpdateEngineTest
 * @tc.number: UpdateEngineTest_001
 * @tc.desc  : Test For Wptr
 */
HWTEST_F(UpdateEngineTest, UpdateEngineTest_001, TestSize.Level1)
{
    sptr<TestWptr> t = sptr<TestWptr>(new TestWptr());
    t->Func();
    t = nullptr;
    ASSERT_EQ(nullptr, t);
}
}  // namespace IntellVoiceTests
}  // namespace OHOS