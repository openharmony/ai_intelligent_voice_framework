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
#include "update_engine_utils.h"

#include <fstream>
#include "securec.h"
#include "intell_voice_log.h"
#include "string_util.h"
#include "history_info_mgr.h"

#define LOG_TAG "UpdateEngineUtils"

using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {
const std::string INTELL_VOICE_DEFAULT_RES_PATH = "/sys_prod/variant/region_comm/china/etc/intellvoice/wakeup/";
const std::string INTELL_VOICE_CONDICT_PATH_DSP = "/dsp/condict/";
static constexpr uint32_t WAKEUP_VERSION_SPLIT_NUM = 2;
static constexpr uint32_t WAKEUP_VERSION_NUMBERS = 3;

UpdateEngineUtils::UpdateEngineUtils()
{
}

UpdateEngineUtils::~UpdateEngineUtils()
{
}

void UpdateEngineUtils::GetWakeupVesion(std::string &versionNumber)
{
    std::string filePath = INTELL_VOICE_DEFAULT_RES_PATH + INTELL_VOICE_CONDICT_PATH_DSP + "version.txt";
    std::string wakeupVersion;
    std::ifstream file(filePath);

    INTELL_VOICE_LOG_INFO("enter");
    if (!file.good()) {
        INTELL_VOICE_LOG_ERROR("Open file(%{public}s) failed", filePath.c_str());
        return;
    }

    getline(file, wakeupVersion);

    if (wakeupVersion.empty()) {
        INTELL_VOICE_LOG_ERROR("wakeupVersion empty");
        return;
    }

    INTELL_VOICE_LOG_INFO("wakeupVersion: (%{public}s)", wakeupVersion.c_str());

    std::vector<std::string> tokens;
    StringUtil::Split(wakeupVersion, "v.", tokens);
    if (tokens.size() != WAKEUP_VERSION_SPLIT_NUM || tokens[1].empty()) {
        INTELL_VOICE_LOG_INFO("wakeupVersion split empty (%{public}zu)", tokens.size());
        return;
    }
    /* 5.2.1 -> 050201 */
    std::string version = tokens[1];
    tokens.resize(0);
    StringUtil::Split(version, ".", tokens);
    if (tokens.size() != WAKEUP_VERSION_NUMBERS) {
        INTELL_VOICE_LOG_INFO("vesion split empty");
        return;
    }

    for (size_t index = 0; index < tokens.size(); index++) {
        if (tokens[index].length() == 1) {
            tokens[index] = "0" + tokens[index];
        }
        versionNumber += tokens[index];
    }
    INTELL_VOICE_LOG_INFO("exit (%{public}s)", versionNumber.c_str());
}

void UpdateEngineUtils::SaveWakeupVesion()
{
    std::string versionNumber;
    HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();

    GetWakeupVesion(versionNumber);
    if (versionNumber.empty()) {
        return;
    }

    historyInfoMgr.SetWakeupVesion(versionNumber);
}

bool UpdateEngineUtils::IsVersionUpdate()
{
    std::string versionNumberSave;
    std::string versionNumberCur;
    HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();

    versionNumberSave = historyInfoMgr.GetWakeupVesion();
    if (versionNumberSave.empty()) {
        INTELL_VOICE_LOG_ERROR("versionNumberSave is null");
        return false;
    }

    GetWakeupVesion(versionNumberCur);
    if (versionNumberCur.empty()) {
        INTELL_VOICE_LOG_ERROR("versionNumberCur is null");
        return false;
    }

    if (stoi(versionNumberCur) > stoi(versionNumberSave)) {
        INTELL_VOICE_LOG_INFO("version new %{public}d cur %{public}d",
            stoi(versionNumberCur), stoi(versionNumberSave));
        return true;
    }

    return false;
}
}
}