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
#include "audio_source.h"

#include "securec.h"
#include "intell_voice_log.h"
#include "memory_guard.h"

#define LOG_TAG "AudioSource"

using namespace OHOS::AudioStandard;
using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {
static const std::string CACHE_PATH = "/data/data/intell_voice/cache/";

AudioSource::AudioSource(uint32_t minBufferSize, uint32_t bufferCnt,
    std::unique_ptr<AudioSourceListener> listener, const OHOS::AudioStandard::AudioCapturerOptions &capturerOptions)
    : minBufferSize_(minBufferSize), bufferCnt_(bufferCnt), listener_(std::move(listener))
{
    capturerOptions_.streamInfo.samplingRate = capturerOptions.streamInfo.samplingRate;
    capturerOptions_.streamInfo.encoding = capturerOptions.streamInfo.encoding;
    capturerOptions_.streamInfo.format = capturerOptions.streamInfo.format;
    capturerOptions_.streamInfo.channels = capturerOptions.streamInfo.channels;
    capturerOptions_.capturerInfo.sourceType = capturerOptions.capturerInfo.sourceType;
    capturerOptions_.capturerInfo.capturerFlags = capturerOptions.capturerInfo.capturerFlags;

    audioCapturer_ = AudioCapturer::Create(capturerOptions_, CACHE_PATH);
    if (audioCapturer_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("create audio capturer failed");
    }
}

AudioSource::~AudioSource()
{
    Stop();
}

bool AudioSource::Start()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (audioCapturer_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("audioCapturer_ is nullptr");
        return false;
    }

    if (listener_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("listener_ is nullptr");
        return false;
    }

    if (minBufferSize_ == 0) {
        INTELL_VOICE_LOG_ERROR("minBufferSize_ is invalid");
        return false;
    }

    buffer_ = std::shared_ptr<uint8_t>(new uint8_t[minBufferSize_], [](uint8_t *p) { delete[] p; });
    if (buffer_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("malloc buffer failed");
        return false;
    }

    CreateAudioDebugFile();

    if (!audioCapturer_->Start()) {
        INTELL_VOICE_LOG_ERROR("start audio capturer failed");
        DestroyAudioDebugFile();
        return false;
    }

    isReading_.store(true);
    std::thread t1(std::bind(&AudioSource::ReadThread, this));
    readThread_ = std::move(t1);
    return true;
}

void AudioSource::ReadThread()
{
    INTELL_VOICE_LOG_INFO("enter");
    uint32_t readCnt = 0;
    bool isError = true;
    while (isReading_.load()) {
        if (readCnt >= bufferCnt_) {
            INTELL_VOICE_LOG_INFO("finish reading data");
            isError = false;
            break;
        }

        if (!Read()) {
            INTELL_VOICE_LOG_WARN("failed to read data");
            break;
        }
        ++readCnt;
    }

    if (listener_ != nullptr) {
        listener_->bufferEndCb_(isError);
    }
}

bool AudioSource::Read()
{
    size_t bytesRead = 0;
    while (bytesRead < minBufferSize_) {
        if (!isReading_.load()) {
            INTELL_VOICE_LOG_WARN("stop to read");
            break;
        }
        int32_t len = audioCapturer_->Read(*(buffer_.get() + bytesRead), minBufferSize_ - bytesRead, 0);
        if (len >= 0) {
            bytesRead += static_cast<uint32_t>(len);
        } else {
            INTELL_VOICE_LOG_ERROR("read data error, len is %{public}d", len);
            break;
        }
    }

    if (bytesRead != minBufferSize_) {
        INTELL_VOICE_LOG_ERROR("failed to read data,  bytesRead is %{public}zu", bytesRead);
        return false;
    }

    WriteData(reinterpret_cast<char *>(buffer_.get()), minBufferSize_);

    if (listener_ != nullptr) {
        listener_->readBufferCb_(buffer_.get(), minBufferSize_);
    }
    return true;
}

void AudioSource::Stop()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (!isReading_.load()) {
        INTELL_VOICE_LOG_INFO("already stop");
        return;
    }

    MemoryGuard memoryGuard;

    isReading_.store(false);
    readThread_.join();

    DestroyAudioDebugFile();

    if (audioCapturer_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("audioCapturer_ is nullptr");
        return;
    }

    if (!audioCapturer_->Stop()) {
        INTELL_VOICE_LOG_ERROR("stop audio capturer error");
    }

    if (!audioCapturer_->Release()) {
        INTELL_VOICE_LOG_ERROR("release audio capturer error");
    }

    audioCapturer_ = nullptr;
    listener_ = nullptr;
}
}
}