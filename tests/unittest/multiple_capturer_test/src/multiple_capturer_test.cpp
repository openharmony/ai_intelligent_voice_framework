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
#include "multiple_capturer_test.h"

#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
std::mutex engineMutex_;

void MultipleAudioCapturerUnitTest::SetUpTestCase(void) {}
void MultipleAudioCapturerUnitTest::TearDownTestCase(void) {}
void MultipleAudioCapturerUnitTest::SetUp(void) {}
void MultipleAudioCapturerUnitTest::TearDown(void) {}

void MultipleAudioCapturerUnitTest::InitIntellVoice(sptr<IntellVoiceEngine::EngineBase> &engine)
{
    std::lock_guard<std::mutex> lock(engineMutex_);
    engine = IntellVoiceEngine::EngineFactory::CreateEngineInst(IntellVoiceEngine::INTELL_VOICE_WAKEUP, "");
    ASSERT_NE(nullptr, engine);

    return;
}

void MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(unique_ptr<AudioCapturer> &audioCapturer,
    shared_ptr<MultipleAudioCapturerCallbackTest> &cb, SourceType sourceType, bool isStarted)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = sourceType;
    audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    cb = make_shared<MultipleAudioCapturerCallbackTest>();
    ASSERT_NE(nullptr, cb);
    int32_t ret = audioCapturer->SetCapturerCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    bool startOK = audioCapturer->Start();
    EXPECT_EQ(isStarted, startOK);

    return;
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_001, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    if (audioCapturer1 != nullptr) audioCapturer1->Release();
    if (audioCapturer2 != nullptr) audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_003, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_004, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_005, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_006, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_007, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_008, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_009, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_010, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_011, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_012, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_013, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_014, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_015, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_016, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_017, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_018, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_019, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_020, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_021, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_022, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_023, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_024, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_025, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_026, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_027, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_028, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_029, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_030, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_031, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_032, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_033, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_034, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_035, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_036, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_037, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_038, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_039, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_040, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_041, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine1 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine2 = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine1);
    int32_t wakeupStart1 = wakeupEngine1->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart1);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine2);
    int32_t wakeupStart2 = wakeupEngine2->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart2);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer, cb, SOURCE_TYPE_MIC);

    audioCapturer->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_042, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine1 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine2 = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine1);
    int32_t wakeupStart1 = wakeupEngine1->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart1);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine2);
    int32_t wakeupStart2 = wakeupEngine2->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart2);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer, cb, SOURCE_TYPE_VOICE_RECOGNITION);

    audioCapturer->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_043, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine1 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine2 = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine1);
    int32_t wakeupStart1 = wakeupEngine1->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart1);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine2);
    int32_t wakeupStart2 = wakeupEngine2->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart2);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer, cb, SOURCE_TYPE_VOICE_COMMUNICATION);

    audioCapturer->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_044, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine1 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine2 = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine1);
    int32_t wakeupStart1 = wakeupEngine1->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart1);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine2);
    int32_t wakeupStart2 = wakeupEngine2->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart2);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer, cb, SOURCE_TYPE_ULTRASONIC);

    audioCapturer->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_045, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine1 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine2 = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine1);
    int32_t wakeupStart1 = wakeupEngine1->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart1);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine2);
    int32_t wakeupStart2 = wakeupEngine2->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart2);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer, cb, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_046, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_047, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_048, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_049, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine1 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine2 = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine1);
    int32_t wakeupStart1 = wakeupEngine1->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart1);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer, cb, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine2);
    int32_t wakeupStart2 = wakeupEngine2->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart2);

    audioCapturer->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_050, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_051, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_052, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_053, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_054, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine1 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine2 = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine1);
    int32_t wakeupStart1 = wakeupEngine1->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart1);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer, cb, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine2);
    int32_t wakeupStart2 = wakeupEngine2->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart2);

    audioCapturer->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_055, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_056, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_057, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_058, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_059, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine1 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine2 = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine1);
    int32_t wakeupStart1 = wakeupEngine1->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart1);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer, cb, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine2);
    int32_t wakeupStart2 = wakeupEngine2->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart2);

    audioCapturer->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_060, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_061, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_062, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_063, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_064, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine1 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine2 = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine1);
    int32_t wakeupStart1 = wakeupEngine1->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart1);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer, cb, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine2);
    int32_t wakeupStart2 = wakeupEngine2->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart2);

    audioCapturer->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_065, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_066, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_067, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_068, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_069, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_070, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine1 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine2 = nullptr;

    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine1);
    int32_t wakeupStart1 = wakeupEngine1->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart1);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer, cb, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine2);
    int32_t wakeupStart2 = wakeupEngine2->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart2);

    audioCapturer->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_071, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_072, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_073, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_074, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_075, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(200000);
    EXPECT_EQ(-1, wakeupStart);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_076, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_077, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_078, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_079, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
}

HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_080, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    sptr<IntellVoiceEngine::EngineBase> wakeupEngine = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::InitIntellVoice(wakeupEngine);
    int32_t wakeupStart = wakeupEngine->Start(true);
    usleep(3000000);
    EXPECT_EQ(0, wakeupStart);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
}
} // namespace AudioStandard
} // namespace OHOS
