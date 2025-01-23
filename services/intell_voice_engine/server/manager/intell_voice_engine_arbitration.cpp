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
#include "intell_voice_engine_arbitration.h"

#include "intell_voice_log.h"

#define LOG_TAG "EngineArbitration"

using namespace std;

namespace OHOS {
namespace IntellVoiceEngine {
IntellVoiceEngineArbitration::IntellVoiceEngineArbitration()
{
    engineRelationTbl_[INTELL_VOICE_ENROLL][INTELL_VOICE_WAKEUP] = PREEMPTION_TYPE;
    engineRelationTbl_[INTELL_VOICE_ENROLL][INTELL_VOICE_UPDATE] = REPLACEMENT_TYPE;
    engineRelationTbl_[INTELL_VOICE_WAKEUP][INTELL_VOICE_ENROLL] = CONCURRENCY_TYPE;
    engineRelationTbl_[INTELL_VOICE_UPDATE][INTELL_VOICE_ENROLL] = REJECTION_TYPE;
}

IntellVoiceEngineArbitration::~IntellVoiceEngineArbitration()
{
}

ArbitrationResult IntellVoiceEngineArbitration::ApplyArbitration(IntellVoiceEngineType type,
    std::map<IntellVoiceEngineType, sptr<EngineBase>> &engines)
{
    if (JudgeRejection(type, engines)) {
        INTELL_VOICE_LOG_INFO("reject engine, type: %{public}d", type);
        return ARBITRATION_REJECT;
    }

    HandlePreemption(type, engines);
    HandleReplace(type, engines);

    return ARBITRATION_OK;
}

sptr<EngineBase> IntellVoiceEngineArbitration::GetEngine(IntellVoiceEngineType type,
    const std::map<IntellVoiceEngineType, sptr<EngineBase>> &engines)
{
    auto it = engines.find(type);
    if (it != engines.end() && it->second != nullptr) {
        return it->second;
    }

    return nullptr;
}

bool IntellVoiceEngineArbitration::JudgeRejection(IntellVoiceEngineType type,
    const std::map<IntellVoiceEngineType, sptr<EngineBase>> &engines)
{
    auto relation = engineRelationTbl_.find(type);
    if (relation == engineRelationTbl_.end()) {
        return false;
    }

    for (auto it : relation->second) {
        if (it.second != REJECTION_TYPE) {
            continue;
        }
        if (GetEngine(it.first, engines) != nullptr) {
            return true;
        }
    }
    return false;
}

void IntellVoiceEngineArbitration::HandlePreemption(IntellVoiceEngineType type,
    const std::map<IntellVoiceEngineType, sptr<EngineBase>> &engines)
{
    auto relation = engineRelationTbl_.find(type);
    if (relation == engineRelationTbl_.end()) {
        return;
    }

    for (auto it : relation->second) {
        if (it.second != PREEMPTION_TYPE) {
            continue;
        }
        auto engine = GetEngine(it.first, engines);
        if (engine == nullptr) {
            continue;
        }

        engine->Stop();
        INTELL_VOICE_LOG_INFO("preemption, type: %{public}d, %{public}d", type, it.first);
    }
}

void IntellVoiceEngineArbitration::HandleReplace(IntellVoiceEngineType type,
    std::map<IntellVoiceEngineType, sptr<EngineBase>> &engines)
{
    auto relation = engineRelationTbl_.find(type);
    if (relation == engineRelationTbl_.end()) {
        return;
    }

    for (auto it : relation->second) {
        if (it.second != REPLACEMENT_TYPE) {
            continue;
        }

        auto engineIter = engines.find(it.first);
        if ((engineIter == engines.end()) || (engineIter->second == nullptr)) {
            continue;
        }

        engineIter->second->Detach();
        engineIter->second = nullptr;
        engines.erase(it.first);
        INTELL_VOICE_LOG_INFO("replace, type: %{public}d, %{public}d", type, it.first);
    }
}
}
}