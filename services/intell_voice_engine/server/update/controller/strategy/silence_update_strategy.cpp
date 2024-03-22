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
#include "silence_update_strategy.h"
#include <fstream>
#include "securec.h"
#include "intell_voice_log.h"
#include "scope_guard.h"
#include "update_engine_utils.h"
#include "history_info_mgr.h"
#include "ability_manager_client.h"

#define LOG_TAG "SilenceUpdateStrategy"

using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {
static constexpr int32_t SILENCE_UPDATE_RETRY_TIMES = 5;

SilenceUpdateStrategy::SilenceUpdateStrategy(const std::string &param): IUpdateStrategy(param)
{
}

SilenceUpdateStrategy::~SilenceUpdateStrategy()
{
}

bool SilenceUpdateStrategy::UpdateRestrain()
{
    return !UpdateEngineUtils::IsVersionUpdate();
}

UpdatePriority SilenceUpdateStrategy::GetUpdatePriority()
{
    return SILENCE_UPDATE_PRIORITY;
}

int SilenceUpdateStrategy::GetRetryTimes()
{
    return SILENCE_UPDATE_RETRY_TIMES;
}

void SilenceUpdateStrategy::NotifyUpdateFail()
{
    AAFwk::Want want;
    HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();

    std::string bundleName = historyInfoMgr.GetWakeupEngineBundleName();
    std::string abilityName = historyInfoMgr.GetWakeupEngineAbilityName();
    INTELL_VOICE_LOG_INFO("bundleName:%{public}s, abilityName:%{public}s", bundleName.c_str(), abilityName.c_str());
    want.SetElementName(bundleName, abilityName);
    want.SetParam("serviceName", std::string("intell_voice"));
    want.SetParam("servicePid", getpid());
    want.SetParam("eventType", std::string("update_event"));
    AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want);
}

int SilenceUpdateStrategy::OnUpdateCompleteCallback(const int result, bool isLast)
{
    if (!isLast || result == 0) {
        return 0;
    }

    INTELL_VOICE_LOG_INFO("notify silence update fail");
    NotifyUpdateFail();
    return  0;
}
}
}