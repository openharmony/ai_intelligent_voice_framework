/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef WAKEUP_MANAGER_TEST_H
#define WAKEUP_MANAGER_TEST_H

#include <unistd.h>
#include <cstdio>

#include "wakeup_engine_test.h"

namespace OHOS {
namespace IntellVoiceEngine {
class WakeupManagerTest {
public:
    WakeupManagerTest();
    ~WakeupManagerTest();

    int32_t InitRecognize();
    int32_t StartRecognize();
    int32_t StartCapturer();

    std::shared_ptr<WakeupEngineTest> wakeupImpl_ = nullptr;
    std::mutex wakeupMutex_;
};
}  // namespace IntellVoiceTrigger
}  // namespace OHOS
#endif