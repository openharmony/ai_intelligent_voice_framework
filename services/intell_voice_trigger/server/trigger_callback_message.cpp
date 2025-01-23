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
#include "trigger_callback_message.h"
#include "intell_voice_log.h"

#define LOG_TAG "TriggerCallback"
namespace OHOS {
namespace IntellVoiceTrigger {
std::map<TriggerCbMessageId, TriggerCallbackMessage::Func> TriggerCallbackMessage::g_triggerFuncMap;

void TriggerCallbackMessage::RegisterFunc(TriggerCbMessageId id, Func func)
{
    g_triggerFuncMap[id] = func;
}

void TriggerCallbackMessage::CallFunc(TriggerCbMessageId id)
{
    if ((g_triggerFuncMap.count(id) != 0) && (g_triggerFuncMap[id] != nullptr)) {
        g_triggerFuncMap[id]();
    } else {
        INTELL_VOICE_LOG_ERROR("failed to find trigger function with id: %{public}d", static_cast<int>(id));
    }
}
}  // namespace IntellVoicetrigger
}  // namespace OHOS
