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
#ifndef AUDIO_DEBUG_H
#define AUDIO_DEBUG_H

#include <cstdint>
#include <string>
#ifdef AUDIO_DATA_DEBUG
#include <memory>
#include <fstream>
#endif

namespace OHOS {
namespace IntellVoiceEngine {
class AudioDebug {
public:
    AudioDebug() = default;
    ~AudioDebug() = default;

    void CreateAudioDebugFile(const std::string &suffix);
    void WriteData(const char *data, uint32_t length);
    void DestroyAudioDebugFile();

private:
#ifdef AUDIO_DATA_DEBUG
    std::unique_ptr<std::ofstream> fileStream_ = nullptr;
#endif
};
}
}
#endif