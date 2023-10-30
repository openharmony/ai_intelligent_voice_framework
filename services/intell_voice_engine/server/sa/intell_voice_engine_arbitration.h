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

#ifndef INTELL_VOICE_ENGINE_ARBITRATION_H
#define INTELL_VOICE_ENGINE_ARBITRATION_H

#include <map>
#include "engine_base.h"

namespace OHOS {
namespace IntellVoiceEngine {
enum EngineRelationType {
    CONCURRENCY_TYPE = 0,
    REJECTION_TYPE,
    PREEMPTION_TYPE,
    REPLACEMENT_TYPE,
};

enum ArbitrationResult {
    ARBITRATION_OK = 0,
    ARBITRATION_REJECT,
};

class IntellVoiceEngineArbitration {
public:
    IntellVoiceEngineArbitration();
    ~IntellVoiceEngineArbitration();

    ArbitrationResult ApplyArbitration(IntellVoiceEngineType type,
        std::map<IntellVoiceEngineType, sptr<EngineBase>> &engines);
    sptr<EngineBase> GetEngine(IntellVoiceEngineType type,
        const std::map<IntellVoiceEngineType, sptr<EngineBase>> &engines);

private:
    bool JudgeRejection(IntellVoiceEngineType type, const std::map<IntellVoiceEngineType, sptr<EngineBase>> &engines);
    void HandlePreemption(IntellVoiceEngineType type,
        const std::map<IntellVoiceEngineType, sptr<EngineBase>> &engines);
    void HandleReplace(IntellVoiceEngineType type, std::map<IntellVoiceEngineType, sptr<EngineBase>> &engines);

private:
    std::map<IntellVoiceEngineType, std::map<IntellVoiceEngineType, EngineRelationType>> engineRelationTbl_;
};
}
}
#endif

