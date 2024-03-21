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
#include "clone_update_strategy.h"
#include <fstream>
#include "securec.h"
#include "intell_voice_log.h"
#include "scope_guard.h"
#include "update_engine_utils.h"
#include "history_info_mgr.h"

#define LOG_TAG "CloneUpdateStrategy"

using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {
CloneUpdateStrategy::CloneUpdateStrategy(const std::string &param,
    sptr<IIntelligentVoiceUpdateCallback> updateCallback): IUpdateStrategy(param), updateCallback_(updateCallback)
{
}

CloneUpdateStrategy::~CloneUpdateStrategy()
{
}

bool CloneUpdateStrategy::UpdateRestrain()
{
    std::string versionNumberSave;
    HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();

    versionNumberSave = historyInfoMgr.GetWakeupVesion();
    if (versionNumberSave.empty()) {
        INTELL_VOICE_LOG_ERROR("saved version number is null");
        return false; // only for test, return true
    }
    return true;  // only for test, return false;
}

UpdatePriority CloneUpdateStrategy::GetUpdatePriority()
{
    return CLONE_UPDATE_PRIORITY;
}

int CloneUpdateStrategy::GetRetryTimes()
{
    return 0;
}

int CloneUpdateStrategy::OnUpdateCompleteCallback(const int result, bool isLast)
{
    if (updateCallback_ != nullptr) {
        INTELL_VOICE_LOG_INFO("enter");
        updateCallback_->OnUpdateComplete(result);
    }

    return  0;
}
}
}