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
#ifndef HISTORY_INFO_MGR_H
#define HISTORY_INFO_MGR_H

#include <vector>
#include "service_db_helper.h"
#include "nocopyable.h"

namespace OHOS {
namespace IntellVoiceEngine {
const std::string KEY_WAKEUP_ENGINE_BUNDLE_NAME = "WakeupEngineBundleName";
const std::string KEY_WAKEUP_ENGINE_ABILITY_NAME = "WakeupEngineAbilityName";
const std::string KEY_WAKEUP_VESRION = "WakeupVersion";
const std::string KEY_LANGUAGE = "Language";
const std::string KEY_AREA = "Area";
const std::string KEY_WAKEUP_PHRASE = "WakeupPhrase";

class HistoryInfoMgr : private ServiceDbHelper {
public:
    HistoryInfoMgr();
    ~HistoryInfoMgr() = default;

    static HistoryInfoMgr& GetInstance()
    {
        static HistoryInfoMgr historyInfoMgr;
        return historyInfoMgr;
    }

    void SetEnrollEngineUid(int32_t uid);
    int32_t GetEnrollEngineUid();
    void SetWakeupEngineBundleName(const std::string &bundleName);
    std::string GetWakeupEngineBundleName();
    void SetWakeupEngineAbilityName(const std::string &abilityName);
    std::string GetWakeupEngineAbilityName();
    void SetWakeupVesion(const std::string &wakeupVesion);
    std::string GetWakeupVesion();
    void SetLanguage(const std::string &language);
    std::string GetLanguage();
    void SetArea(const std::string &area);
    std::string GetArea();
    void SetSensibility(const std::string &sensibility);
    std::string GetSensibility();
    void SetWakeupPhrase(const std::string &wakeupPhrase);
    std::string GetWakeupPhrase();
    void SetWakeupDspFeature(const std::string &wakeupDspFeature);
    std::string GetWakeupDspFeature();
    void DeleteKey(const std::vector<std::string> &keyList);

private:
    DISALLOW_COPY_AND_MOVE(HistoryInfoMgr);
};
}
}
#endif