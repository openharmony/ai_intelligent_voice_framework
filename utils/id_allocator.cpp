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

#include "id_allocator.h"
#include "intell_voice_log.h"

using namespace std;

#define LOG_TAG "IdAllocator"

namespace OHOS {
namespace IntellVoiceUtils {
IdAllocator::IdAllocator(int maxTimerNum)
{
    for (int i = 0; i < maxTimerNum; i++) {
        idFlags.push_back(false);
    }
}

int IdAllocator::AllocId()
{
    for (unsigned int i = 0; i < idFlags.size(); i++) {
        if (!idFlags[i]) {
            idFlags[i] = true;
            return i;
        }
    }

    return INVALID_ID;
}

void IdAllocator::ReleaseId(unsigned int id)
{
    if (id >= idFlags.size()) {
        INTELL_VOICE_LOG_ERROR("try to release invalid id %d", id);
        return;
    }
    INTELL_VOICE_LOG_INFO("release id %u", id);
    idFlags[id] = false;
}

void IdAllocator::ClearId()
{
    for (unsigned int i = 0; i < idFlags.size(); i++) {
        idFlags[i] = false;
    }
}
}
}