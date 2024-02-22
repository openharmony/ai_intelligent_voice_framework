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
#ifndef AUDIO_SOURCE_H
#define AUDIO_SOURCE_H

#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include "audio_capturer.h"
#include "audio_info.h"
#include "audio_debug.h"

namespace OHOS {
namespace IntellVoiceEngine {
using OnReadBufferCb = std::function<void(uint8_t *buffer, uint32_t size, bool isEnd)>;
using OnBufferEndCb = std::function<void()>;

struct AudioSourceListener {
    AudioSourceListener(OnReadBufferCb readBufferCb, OnBufferEndCb bufferEndCb)
        : readBufferCb_(readBufferCb), bufferEndCb_(bufferEndCb) {}
    OnReadBufferCb readBufferCb_;
    OnBufferEndCb bufferEndCb_;
};

class AudioSource : private AudioDebug {
public:
    AudioSource(uint32_t minBufferSize, uint32_t bufferCnt, std::unique_ptr<AudioSourceListener> listener,
        const OHOS::AudioStandard::AudioCapturerOptions &capturerOptions);
    ~AudioSource();
    bool Start();
    void Stop();

private:
    void ReadThread();
    bool Read();

private:
    uint32_t minBufferSize_ = 0;
    uint32_t bufferCnt_ = 0;
    bool isEnd_ = false;
    std::unique_ptr<AudioSourceListener> listener_ = nullptr;
    std::atomic<bool> isReading_ = false;
    std::thread readThread_;
    OHOS::AudioStandard::AudioCapturerOptions capturerOptions_;
    std::shared_ptr<uint8_t> buffer_ = nullptr;
    std::unique_ptr<OHOS::AudioStandard::AudioCapturer> audioCapturer_ = nullptr;
};
}
}
#endif