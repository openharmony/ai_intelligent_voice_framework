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
#include "intell_voice_util.h"
#include <memory>
#include "intell_voice_log.h"

#define LOG_TAG "IntellVoiceUtil"

namespace OHOS {
namespace IntellVoiceUtils {
static constexpr uint32_t VERSION_OFFSET = 8;

uint32_t GetHdiVersionId(uint32_t majorVer, uint32_t minorVer)
{
    return ((majorVer << VERSION_OFFSET) | minorVer);
}

bool DeinterleaveAudioData(int16_t *buffer, uint32_t size, int32_t channelCnt,
    std::vector<std::vector<uint8_t>> &audioData)
{
    uint32_t channelLen = size / channelCnt;
    std::unique_ptr<int16_t[]> channelData = std::make_unique<int16_t[]>(channelLen);
    if (channelData == nullptr) {
        INTELL_VOICE_LOG_ERROR("channelData is nullptr");
        return false;
    }
    for (int32_t i = 0; i < channelCnt; i++) {
        for (uint32_t j = 0; j < channelLen; j++) {
            channelData[j] = buffer[i + j * channelCnt];
        }
        std::vector<uint8_t> item(reinterpret_cast<uint8_t *>(channelData.get()),
            reinterpret_cast<uint8_t *>(channelData.get()) + channelLen * sizeof(int16_t));
        audioData.emplace_back(item);
    }
    return true;
}
}
}
