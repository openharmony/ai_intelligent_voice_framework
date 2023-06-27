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

#include "service_db_helper.h"
#include "nocopyable.h"

namespace OHOS {
namespace IntellVoiceEngine {
class HistoryInfoMgr : private ServiceDbHelper {
public:
    HistoryInfoMgr();
    ~HistoryInfoMgr() = default;

    void SetEnrollEngineUid(int32_t uid);
    int32_t GetEnrollEngineUid();
    void SetWakeupEngineBundleName(std::string bundleName);
    std::string GetWakeupEngineBundleName();
    void SetWakeupEngineAbilityName(std::string abilityName);
    std::string GetWakeupEngineAbilityName();

private:
    DISALLOW_COPY_AND_MOVE(HistoryInfoMgr);
};
}
}
#endif