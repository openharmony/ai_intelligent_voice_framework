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
#ifndef STATE_MANAGER_TEST_H
#define STATE_MANAGER_TEST_H

#include <unistd.h>
#include <cstdio>

#include "state_manager.h"

namespace OHOS {
namespace IntellVoiceEngine {
using OHOS::IntellVoiceUtils::State;
using OHOS::IntellVoiceUtils::StateMsg;

enum StateManager {
    FIRST = 0,
    SECOND = 1,
    THIRD = 2,
    FOURTH = 3,
};

enum EventManager {
    EVENT_FIRST = 0,
    EVENT_SECOND,
    EVENT_THIRD,
    EVENT_FOURTH,
    EVENT_FIRTH,
};

class StateManagerTest : private OHOS::IntellVoiceUtils::ModuleStates {
public:
    StateManagerTest();
    ~StateManagerTest();
    int32_t Handle(const StateMsg &msg);
    bool Init();

private:
    bool InitStates();
    int32_t HandleSetParam(const StateMsg &msg, State &nextState);
    int32_t HandleInit(const StateMsg &msg, State &nextState);
    int32_t HandleStart(const StateMsg &msg, State &nextState);
    int32_t HandleRecognizeComplete(const StateMsg &msg, State &nextState);
    int32_t HandleEndRecognize(const StateMsg &msg, State &nextState);
    int32_t HandleRecognizingTimeout(const StateMsg &msg, State &nextState);
};
}  // namespace IntellVoiceEngine
}  // namespace OHOS
#endif