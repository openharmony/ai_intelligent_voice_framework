/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "whisper_update_strategy.h"
#include <fstream>
#include "securec.h"
#include "intell_voice_log.h"
#include "scope_guard.h"
#include "update_engine_utils.h"
#include "history_info_mgr.h"
#include "ability_manager_client.h"
#include "intell_voice_definitions.h"

#define LOG_TAG "WhisperUpdateStrategy"

using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {
static constexpr int32_t WHISPER_UPDATE_RETRY_TIMES = 5;

WhisperUpdateStrategy::WhisperUpdateStrategy(const std::string &param): IUpdateStrategy(param)
{
}

WhisperUpdateStrategy::~WhisperUpdateStrategy()
{
}

bool WhisperUpdateStrategy::UpdateRestrain()
{
    return false;
}

UpdatePriority WhisperUpdateStrategy::GetUpdatePriority()
{
    return WHISPER_UPDATE_PRIORITY;
}

int WhisperUpdateStrategy::GetRetryTimes()
{
    return WHISPER_UPDATE_RETRY_TIMES;
}

void WhisperUpdateStrategy::NotifyUpdateFail()
{
    INTELL_VOICE_LOG_INFO("enter");
    HistoryInfoMgr::GetInstance().SetStringKVPair(KEY_WHISPER_VPR, "false");
}

int WhisperUpdateStrategy::OnUpdateCompleteCallback(const int result, bool isLast)
{
    if (result == 0) {
        HistoryInfoMgr::GetInstance().SetStringKVPair(KEY_WHISPER_VPR, "true");
    }

    if (!isLast || result == 0) {
        return 0;
    }

    INTELL_VOICE_LOG_INFO("notify silence update fail");
    NotifyUpdateFail();
    return  0;
}
}
}