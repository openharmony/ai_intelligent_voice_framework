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
#include "enroll_engine_test.h"

#include "intell_voice_log.h"
#include "enroll_intell_voice_engine.h"

#define LOG_TAG "TestEnrollEngine"

using namespace OHOS::IntellVoice;
using namespace OHOS::IntellVoiceEngine;
using OHOS::IntellVoiceEngine::EvaluationResult;
namespace OHOS {
namespace IntellVoiceFuzzTest {

constexpr int INTELL_ENROLL_TEST_RANDOM_NUM = 11;

static EnrollIntellVoiceEngine *g_enrollEngine = new EnrollIntellVoiceEngine({"小艺小艺"});

class TestEnrollEngineEventCallback : public IIntellVoiceEngineEventCallback {
private:
    void OnEvent(const IntellVoiceEngineCallBackEvent &param) override;
};

void TestEnrollEngineEventCallback::OnEvent(const IntellVoiceEngineCallBackEvent &param)
{}

static void TestInit(const uint8_t *data, size_t sizeIn)
{
    INTELL_VOICE_LOG_INFO("EnrollEngine test Init start");
    EngineConfig config = {"zh", "CN"};
    int32_t ret = g_enrollEngine->Init(config);
    INTELL_VOICE_LOG_INFO("EnrollEngine test Init end, result:%{public}d", ret);
}

static void TestStart(const uint8_t *data, size_t sizeIn)
{
    INTELL_VOICE_LOG_INFO("EnrollEngine test Start start");
    int32_t ret = g_enrollEngine->Start(false);
    INTELL_VOICE_LOG_INFO("EnrollEngine test Start end, result:%{public}d", ret);
}

static void TestStop(const uint8_t *data, size_t sizeIn)
{
    INTELL_VOICE_LOG_INFO("EnrollEngine test Stop start");
    int32_t ret = g_enrollEngine->Stop();
    INTELL_VOICE_LOG_INFO("EnrollEngine test Stop end, result:%{public}d", ret);
}

static void TestCommit(const uint8_t *data, size_t sizeIn)
{
    INTELL_VOICE_LOG_INFO("EnrollEngine test Commit start");
    int32_t ret = g_enrollEngine->Commit();
    INTELL_VOICE_LOG_INFO("EnrollEngine test Commit end, result:%{public}d", ret);
}

static void TestSetSensibility(const uint8_t *data, size_t sizeIn)
{
    INTELL_VOICE_LOG_INFO("EnrollEngine test SetSensibility start");
    int32_t ret = g_enrollEngine->SetSensibility(static_cast<int32_t>(*data));
    INTELL_VOICE_LOG_INFO("EnrollEngine test SetSensibility end, result:%{public}d", ret);
}

static void TestSetWakeupHapInfo(const uint8_t *data, size_t sizeIn)
{
    INTELL_VOICE_LOG_INFO("EnrollEngine test SetSensibility start");
    WakeupHapInfo config = {"com.huawei.hmos.aibase", "WakeUpExtAbility"};
    int32_t ret = g_enrollEngine->SetWakeupHapInfo(config);
    INTELL_VOICE_LOG_INFO("EnrollEngine test SetSensibility end, result:%{public}d", ret);
}

static void TestSetParameter(const uint8_t *data, size_t sizeIn)
{
    INTELL_VOICE_LOG_INFO("EnrollEngine test SetParameter start");
    std::string key = "key";
    std::string value = "123456";
    int32_t ret = g_enrollEngine->SetParameter(key, value);
    INTELL_VOICE_LOG_INFO("EnrollEngine test SetParameter end, result:%{public}d", ret);
}

static void TestGetParameter(const uint8_t *data, size_t sizeIn)
{
    INTELL_VOICE_LOG_INFO("EnrollEngine test GetParameter start");
    std::string key = "key";
    std::string value = g_enrollEngine->GetParameter(key);
    INTELL_VOICE_LOG_INFO("EnrollEngine test GetParameter end, result:%{public}s", value.c_str());
}

static void TestRelease(const uint8_t *data, size_t sizeIn)
{
    INTELL_VOICE_LOG_INFO("EnrollEngine test Release start");
    int32_t ret = g_enrollEngine->Release();
    INTELL_VOICE_LOG_INFO("EnrollEngine test Release end, result:%{public}d", ret);
}

static void TestSetCallback(const uint8_t *data, size_t sizeIn)
{
    INTELL_VOICE_LOG_INFO("EnrollEngine test SetCallback start");
    std::shared_ptr<TestEnrollEngineEventCallback> cb = std::make_shared<TestEnrollEngineEventCallback>();
    int32_t ret = g_enrollEngine->SetCallback(cb);
    INTELL_VOICE_LOG_INFO("EnrollEngine test SetCallback end, result:%{public}d", ret);
}

static void TestEvaluate(const uint8_t *data, size_t sizeIn)
{
    INTELL_VOICE_LOG_INFO("EnrollEngine test Evaluate start");
    EvaluationResult infos;
    int32_t ret = g_enrollEngine->Evaluate(std::to_string(sizeIn), infos);
    INTELL_VOICE_LOG_INFO("EnrollEngine test Evaluate end, result:%{public}d", ret);
}

namespace {
void (*g_enrollFuncTable[INTELL_ENROLL_TEST_RANDOM_NUM])(const uint8_t *, size_t) = {TestInit,
    TestStart,
    TestStop,
    TestCommit,
    TestSetSensibility,
    TestSetWakeupHapInfo,
    TestSetParameter,
    TestGetParameter,
    TestRelease,
    TestSetCallback,
    TestEvaluate};
}
void TestEnrollEngineRandomFuzzer(const uint8_t *data, size_t sizeIn)
{
    INTELL_VOICE_LOG_INFO("TestEnrollRandomFuzzer in");
    if (data == nullptr) {
        return;
    }
    uint8_t type = data[0] % INTELL_ENROLL_TEST_RANDOM_NUM;
    INTELL_VOICE_LOG_INFO("function id: %{public}d", type);
    (*g_enrollFuncTable[type])(data, sizeIn);
    return;
}
}  // namespace IntellVoiceFuzzTest
}  // namespace OHOS