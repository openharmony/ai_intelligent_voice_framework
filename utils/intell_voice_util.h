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
#ifndef INTELL_VOICE_UTIL_H
#define INTELL_VOICE_UTIL_H
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace OHOS {
namespace IntellVoiceUtils {
class IntellVoiceUtil final {
public:
    static uint32_t GetHdiVersionId(uint32_t majorVer, uint32_t minorVer);
    static bool DeinterleaveAudioData(const int16_t *buffer, uint32_t size, int32_t channelCnt,
        std::vector<std::vector<uint8_t>> &audioData);
    static bool VerifySystemPermission(const std::string &permissionName);
    static bool ReadFile(const std::string &filePath, std::shared_ptr<uint8_t> &buffer, uint32_t &size);

private:
    static bool VerifyClientPermission(const std::string &permissionName);
    static bool CheckIsSystemApp();
};
}
}
#endif
