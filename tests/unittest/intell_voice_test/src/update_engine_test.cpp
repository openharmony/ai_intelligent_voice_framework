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

#define LOG_TAG "UpdateEngineTest"

#define INTELL_VOICE_BUILD_VARIANT_ROOT

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace std;
using namespace OHOS::IntellVoiceTests;
using namespace OHOS::IntellVoice;
using namespace OHOS::IntellVoiceEngine;
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
 * @tc.desc  : Test For SilenceUpdate
 */
HWTEST_F(UpdateEngineTest, UpdateEngineTest_001, TestSize.Level1)
{
    if (!EngineHostManager::GetInstance().Init()) {
        INTELL_VOICE_LOG_ERROR("init engine host failed");
    }
    HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();
    historyInfoMgr.SetWakeupVesion("0");
    auto &mgr = IntellVoiceServiceManager::GetInstance();
    mgr->HandleReleaseEngine(INTELL_VOICE_WAKEUP);
    std::shared_ptr<SilenceUpdateStrategy> silenceStrategy = std::make_shared<SilenceUpdateStrategy>("");
    if (silenceStrategy == nullptr) {
        INTELL_VOICE_LOG_ERROR("silence strategy is nullptr");
    }
    std::shared_ptr<IUpdateStrategy> strategy = std::dynamic_pointer_cast<IUpdateStrategy>(silenceStrategy);
    int ret = mgr->CreateUpdateEngineUntilTime(strategy);
    sleep(5);
    EXPECT_EQ(0, ret);
    INTELL_VOICE_LOG_INFO("end");
}

/**
 * @tc.name  : Test Template UpdateEngineTest
 * @tc.number: UpdateEngineTest_001
 * @tc.desc  : Test For Voiceprint Clone And ClearUserData
 */
HWTEST_F(UpdateEngineTest, UpdateEngineTest_002, TestSize.Level1)
{
    std::vector<WakeupSourceFile> cloneFileInfo;
    int ret1 = IntellVoiceManager::GetInstance()->GetWakeupSourceFiles(cloneFileInfo);
    sleep(2);
    EXPECT_EQ(0, ret1);
    INTELL_VOICE_LOG_INFO("GetWakeupSourceFiles end");

    int ret2 = IntellVoiceManager::GetInstance()->ClearUserData();
    sleep(2);
    EXPECT_EQ(0, ret2);
    INTELL_VOICE_LOG_INFO("ClearUserData end");

    Json::Value wakeupInfo;
    wakeupInfo["clone"] = "true";
    wakeupInfo["clone_path"] = "/data/service/el0/intelligent_voice/wakeup/clone";
    wakeupInfo["source_device"] = "BRA-AL00";
    wakeupInfo["source_version"] = "3.0.0.28";
    wakeupInfo["destination_device"] = "${result.substring(0, 8)}";
    wakeupInfo["destination_version"] = "${result.slice(9)}";
    wakeupInfo["bundle_name"] = "com.huawei.hmos.aibase";
    wakeupInfo["ability_name"] = "WakeUpExtAbility";

    cb_ = std::make_shared<IntellVoiceUpdateCallback>();
    int ret3 = IntellVoiceManager::GetInstance()->EnrollWithWakeupFilesForResult(
        cloneFileInfo, wakeupInfo.toStyledString(), cb_);
    sleep(5);
    EXPECT_EQ(0, ret3);
    INTELL_VOICE_LOG_INFO("EnrollWithWakeupFilesForResult end");
}

/**
 * @tc.name  : Test Template UpdateEngineTest
 * @tc.number: UpdateEngineTest_001
 * @tc.desc  : Test For Wptr
 */
HWTEST_F(UpdateEngineTest, UpdateEngineTest_003, TestSize.Level1)
{
    sptr<TestWptr> t = sptr<TestWptr>(new TestWptr());
    t->Func();
    t = nullptr;
    ASSERT_EQ(nullptr, t);
}
}  // namespace IntellVoiceTests
}  // namespace OHOS