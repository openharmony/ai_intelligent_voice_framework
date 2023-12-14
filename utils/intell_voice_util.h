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
#include <functional>

namespace OHOS {
namespace IntellVoiceUtils {
enum class VersionCheckType {
    SMALLER = 0,
    EQUAL = 1,
    GREATER = 2,
};

std::function<VersionCheckType(uint32_t, uint32_t)> CheckHdiVersion(uint32_t majorVerBase, uint32_t minorVerBase)
{
    return [=](uint32_t majorVer, uint32_t minorVer) {
        if (majorVer > majorVerBase) {
            return VersionCheckType::GREATER;
        }

        if (majorVer < majorVerBase) {
            return VersionCheckType::SMALLER;
        }

        if (minorVer > minorVerBase) {
            return VersionCheckType::GREATER;
        }

        if (minorVer < minorVerBase) {
            return VersionCheckType::SMALLER;
        }

        return VersionCheckType::EQUAL;
    };
}
}
}
#endif
