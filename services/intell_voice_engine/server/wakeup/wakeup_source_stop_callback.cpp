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
#include "wakeup_source_stop_callback.h"
#include "intell_voice_log.h"
#include "intell_voice_engine_manager.h"
#include "engine_callback_message.h"

#define LOG_TAG "WakeupSourceStopCallback"

namespace OHOS {
namespace IntellVoiceEngine {
void WakeupSourceStopCallback::OnWakeupClose()
{
    INTELL_VOICE_LOG_INFO("enter");
    EngineCallbackMessage::CallFunc(HANDLE_CLOSE_WAKEUP_SOURCE, false);
}
}
}
