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
#ifndef TRIGGER_CALLBACK_MESSAGE_H
#define TRIGGER_CALLBACK_MESSAGE_H

#include <functional>
#include <map>
#include <string>
#include "nocopyable.h"

namespace OHOS {
namespace IntellVoiceTrigger {
enum TriggerCbMessageId {
    TRIGGER_CONNECT_SERVICE_START = 0,
};

class TriggerCallbackMessage {
public:
    TriggerCallbackMessage() = default;
    ~TriggerCallbackMessage() = default;

    using Func = std::function<void()>;
    static void RegisterFunc(TriggerCbMessageId id, Func func);
    static void CallFunc(TriggerCbMessageId id);

private:
    static std::map<TriggerCbMessageId, Func> g_triggerFuncMap;
    DISALLOW_COPY_AND_MOVE(TriggerCallbackMessage);
};
}  // namespace IntellVoiceTrigger
}  // namespace OHOS
#endif
