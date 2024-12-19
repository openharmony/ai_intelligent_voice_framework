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
#ifndef WAKEUP_ENGINE_TEST_H
#define WAKEUP_ENGINE_TEST_H

#include "wakeup_engine_impl.h"

namespace OHOS {
namespace IntellVoiceEngine {
using OHOS::IntellVoiceUtils::StateMsg;
using OHOS::IntellVoiceUtils::State;

enum EngineEventTest {
    EVENT_NONE = 100,
    EVENT_INIT,
    EVENT_INIT_DONE,
    EVENT_START_RECOGNIZE,
    EVENT_STOP_RECOGNIZE,
    EVENT_RECOGNIZE_COMPLETE,
    EVENT_START_CAPTURER,
    EVENT_READ,
    EVENT_STOP_CAPTURER,
    EVENT_RECOGNIZING_TIMEOUT,
    EVENT_RECOGNIZE_COMPLETE_TIMEOUT,
    EVENT_READ_CAPTURER_TIMEOUT,
    EVENT_SET_LISTENER,
    EVENT_SET_PARAM,
    EVENT_GET_PARAM,
    EVENT_GET_WAKEUP_PCM,
    EVENT_RELEASE_ADAPTER,
    EVENT_RESET_ADAPTER,
    EVENT_RELEASE,
};

class WakeupEngineTest : private WakeupEngineImpl {
public:
    WakeupEngineTest();
    ~WakeupEngineTest();
    int32_t InitState();
    int32_t HandleInitTest(const StateMsg &msg, State &nextState);
    int32_t HandleStartTest(const StateMsg &msg, State &nextState);
    int32_t HandleStartCapturerTest(const StateMsg &msg, State &nextState);
};
}
}
#endif