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
#include "intell_voice_manager_test.h"

#include "intell_voice_manager.h"
#include "intell_voice_log.h"
#include "i_intell_voice_engine.h"
#include "i_intell_voice_engine_callback.h"

#define LOG_TAG "TestIntellVoiceManager"

constexpr int INTELL_VOICE_MANAGER_RANDOM_NUM = 11;

using namespace std;
using namespace OHOS;
using namespace OHOS::IntellVoiceEngine;
using namespace OHOS::IntellVoice;
namespace OHOS {
namespace IntellVoiceFuzzTest {

static void TestCreateEngine(const uint8_t *data, size_t sizeIn)
{
    IntellVoiceEngineType type = static_cast<const IntellVoiceEngineType>(data[0]);
    sptr<IIntellVoiceEngine> engine;
    auto manager = IntellVoiceManager::GetInstance();
    manager->CreateIntellVoiceEngine(type, engine);
}

static void TestReleaseEngine(const uint8_t *data, size_t sizeIn)
{
    IntellVoiceEngineType type = static_cast<const IntellVoiceEngineType>(data[0]);
    sptr<IIntellVoiceEngine> engine;
    auto manager = IntellVoiceManager::GetInstance();
    manager->ReleaseIntellVoiceEngine(type);
}

static void TestCreateHeadsetEngine(const uint8_t *data, size_t sizeIn)
{
    auto manager = IntellVoiceManager::GetInstance();
    manager->CreateHeadsetWakeupEngine();
}

static void TestRegisterRecipient(const uint8_t *data, size_t sizeIn)
{
    auto manager = IntellVoiceManager::GetInstance();
    sptr<OHOS::IRemoteObject::DeathRecipient> callback;
    manager->RegisterServiceDeathRecipient(callback);
}

static void TestDeregisterRecipient(const uint8_t *data, size_t sizeIn)
{
    auto manager = IntellVoiceManager::GetInstance();
    sptr<OHOS::IRemoteObject::DeathRecipient> callback;
    manager->DeregisterServiceDeathRecipient(callback);
}

static void TestGetUploadFiles(const uint8_t *data, size_t sizeIn)
{
    auto manager = IntellVoiceManager::GetInstance();
    std::vector<UploadFilesInfo> files;
    manager->GetUploadFiles(sizeIn, files);
}

static void TestSetParameter(const uint8_t *data, size_t sizeIn)
{
    auto manager = IntellVoiceManager::GetInstance();
    manager->SetParameter(to_string(data[0]), to_string(sizeIn));
}

static void TestGetParameter(const uint8_t *data, size_t sizeIn)
{
    auto manager = IntellVoiceManager::GetInstance();
    manager->GetParameter(to_string(sizeIn));
}

static void TestGetWakeupSourceFiles(const uint8_t *data, size_t sizeIn)
{
    auto manager = IntellVoiceManager::GetInstance();
    std::vector<WakeupSourceFile> cloneFileInfo;
    manager->GetWakeupSourceFiles(cloneFileInfo);
}

static void TestEnrollWithWakeupFilesForResult(const uint8_t *data, size_t sizeIn)
{
    auto manager = IntellVoiceManager::GetInstance();
    std::vector<WakeupSourceFile> cloneFileInfo;
    WakeupSourceFile file = {"test", {1, 2, 3, 4, 5}};
    cloneFileInfo.push_back(file);
    std::string wakeupInfo = "test";
    shared_ptr<IIntellVoiceUpdateCallback> callback;
    manager->EnrollWithWakeupFilesForResult(cloneFileInfo, wakeupInfo, callback);
}

static void TestClearUserData(const uint8_t *data, size_t sizeIn)
{
    auto manager = IntellVoiceManager::GetInstance();
    manager->ClearUserData();
}

namespace {
void (*g_intellvoicemanagerFuncTable[INTELL_VOICE_MANAGER_RANDOM_NUM])(const uint8_t *, size_t) = {
    TestCreateEngine,
    TestReleaseEngine,
    TestCreateHeadsetEngine,
    TestRegisterRecipient,
    TestDeregisterRecipient,
    TestGetUploadFiles,
    TestSetParameter,
    TestGetParameter,
    TestGetWakeupSourceFiles,
    TestEnrollWithWakeupFilesForResult,
    TestClearUserData
};
}
void TestIntellVoiceRandomFuzzer(const uint8_t *data, size_t sizeIn)
{
    INTELL_VOICE_LOG_INFO("TestIntellVoiceRandomFuzzer in");
    uint8_t type = data[0] % INTELL_VOICE_MANAGER_RANDOM_NUM;  // get the radom num to call the interface
    INTELL_VOICE_LOG_INFO("intell voice manager type: %{public}d", type);
    (*g_intellvoicemanagerFuncTable[type])(data, sizeIn);
    return;
}
}  // namespace IntellVoiceFuzzTest
}  // namespace OHOS