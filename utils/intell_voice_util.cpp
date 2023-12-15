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

namespace OHOS {
namespace IntellVoiceUtils {
static constexpr uint32_t VERSION_OFFSET = 8;

uint32_t GetHdiVersionId(uint32_t majorVer, uint32_t minorVer)
{
    return ((majorVer << VERSION_OFFSET) | minorVer);
}
}
}
