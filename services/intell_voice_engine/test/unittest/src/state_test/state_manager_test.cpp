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

#include "state_manager_test.h"

#include "intell_voice_log.h"

#define LOG_TAG "StateManagerTest"

using namespace std;
using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {

static constexpr int64_t RECOGNIZING_TIMEOUT_US = 2 * 1000 * 1000;  // 10s

StateManagerTest::StateManagerTest() : ModuleStates(State(FIRST), "StateManagerTest", "StateTestThread") {}

StateManagerTest::~StateManagerTest() {}

bool StateManagerTest::Init()
{
    if (!InitStates()) {
        INTELL_VOICE_LOG_ERROR("init state failed");
        return false;
    }
    return true;
}

bool StateManagerTest::InitStates()
{
    ForState(FIRST).ACT(EVENT_SECOND, HandleInit);

    ForState(SECOND).ACT(EVENT_THIRD, HandleStart);

    ForState(THIRD).ACT(EVENT_FOURTH, HandleRecognizeComplete);

    ForState(FOURTH).ACT(EVENT_FIRTH, HandleEndRecognize);

    return IsStatesInitSucc();
}

int32_t StateManagerTest::Handle(const StateMsg &msg)
{
    if (!IsStatesInitSucc()) {
        INTELL_VOICE_LOG_ERROR("failed to init state");
        return -1;
    }

    return ModuleStates::HandleMsg(msg);
}

int32_t StateManagerTest::HandleInit(const StateMsg &msg, State &nextState)
{
    nextState = State(SECOND);
    return 0;
}
int32_t StateManagerTest::HandleStart(const StateMsg &msg, State &nextState)
{
    nextState = State(THIRD);
    return 0;
}
int32_t StateManagerTest::HandleRecognizeComplete(const StateMsg &msg, State &nextState)
{
    nextState = State(FOURTH);
    return 0;
}

int32_t StateManagerTest::HandleEndRecognize(const StateMsg &msg, State &nextState)
{
    nextState = State(FIRST);
    return 0;
}

}  // namespace IntellVoiceEngine
}  // namespace OHOS
