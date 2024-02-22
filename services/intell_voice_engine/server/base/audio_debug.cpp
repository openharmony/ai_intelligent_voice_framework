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
#include "audio_debug.h"
#ifdef AUDIO_DATA_DEBUG
#include "intell_voice_log.h"
#include "time_util.h"

#define LOG_TAG "AudioDebug"
#endif

namespace OHOS {
namespace IntellVoiceEngine {
#ifdef AUDIO_DATA_DEBUG

static const std::string PCM_DIR = "/data/data/intell_voice/pcm_data/";

void AudioDebug::CreateAudioDebugFile(const std::string &suffix)
{
    DestroyAudioDebugFile();

    auto path = PCM_DIR + OHOS::IntellVoiceUtils::TimeUtil::GetCurrTime() + suffix+ ".pcm";
    fileStream_ = std::make_unique<std::ofstream>(path);
    if (fileStream_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("open debug record file failed");
        return;
    }
}

void AudioDebug::WriteData(const char *data, uint32_t length)
{
    if (fileStream_ != nullptr) {
        fileStream_->write(data, length);
    }
}

void AudioDebug::DestroyAudioDebugFile()
{
    if (fileStream_ != nullptr) {
        fileStream_->close();
        fileStream_ = nullptr;
    }
}

#else

void AudioDebug::CreateAudioDebugFile(const std::string & /* suffix */)
{
}

void AudioDebug::WriteData(const char * /* data */, uint32_t /* length */)
{
}

void AudioDebug::DestroyAudioDebugFile()
{
}
#endif
}
}
