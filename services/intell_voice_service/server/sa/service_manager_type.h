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
#ifndef SERVICE_MANAGER_TYPE_H
#define SERVICE_MANAGER_TYPE_H

#ifdef TRIGGER_ENABLE
#include "trigger_manager.h"
#else
#include "dummy_trigger_manager.h"
#endif

#ifdef ENGINE_ENABLE
#include "intell_voice_engine_manager.h"
#elif defined(FIRST_STAGE_ONESHOT_ENABLE)
#include "only_first_engine_manager.h"
#else
#include "dummy_engine_manager.h"
#endif

#include "ffrt_api.h"

namespace OHOS {
namespace IntellVoiceEngine {
template<typename T, typename E>
class IntellVoiceServiceManager;
#ifdef TRIGGER_ENABLE
using TriggerManagerType = OHOS::IntellVoiceTrigger::TriggerManager;
#else
using TriggerManagerType = OHOS::IntellVoiceTrigger::DummyTriggerManager;
#endif
#ifdef ENGINE_ENABLE
using EngineManagerType = IntellVoiceEngineManager;
#elif defined(FIRST_STAGE_ONESHOT_ENABLE)
using EngineManagerType = OnlyFirstEngineManager;
#else
using EngineManagerType = DummyEngineManager;
#endif
using ServiceManagerType = IntellVoiceServiceManager<TriggerManagerType, EngineManagerType>;
}  // namespace IntellVoice
}  // namespace OHOS
#endif
