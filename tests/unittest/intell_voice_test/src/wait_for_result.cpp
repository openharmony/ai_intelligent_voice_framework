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
#include "wait_for_result.h"
#include "intell_voice_log.h"

#define LOG_TAG "WaitForResult"

namespace OHOS {
namespace IntellVoiceTests {
void WaitForResult::Wait()
{
    std::unique_lock<std::mutex> lock(mtx);
    condVar.wait(lock, [this] {return isReady;});
}

void WaitForResult::SetIsReady()
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        isReady = true;
    }
    INTELL_VOICE_LOG_INFO("notify_one");
    condVar.notify_one();
}
}
}
