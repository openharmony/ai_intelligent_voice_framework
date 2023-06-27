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

#include "engine_factory.h"
#include "enroll_engine.h"
#include "wakeup_engine.h"
#include "intell_voice_log.h"
#include "intell_voice_generic_factory.h"

using namespace OHOS::IntellVoiceUtils;
#define LOG_TAG "EngineFactory"

namespace OHOS {
namespace IntellVoiceEngine {
sptr<EngineBase> EngineFactory::CreateEngineInst(IntellVoiceEngineType type)
{
    sptr<EngineBase> engine = nullptr;
    switch (type) {
        case INTELL_VOICE_ENROLL:
            engine = SptrFactory<EnrollEngine>::CreateInstance();
            break;
        case INTELL_VOICE_WAKEUP:
            engine = SptrFactory<WakeupEngine>::CreateInstance();
            break;
        case INTELL_VOICE_UPDATE:
        default:
            INTELL_VOICE_LOG_INFO("create engine enter, type:%d", type);
            break;
    }

    return engine;
}
}
}
