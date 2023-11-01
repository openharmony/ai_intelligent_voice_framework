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
#include "history_info_mgr.h"

#include "string_util.h"

#define LOG_TAG "HistoryInfoMgr"

using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {
constexpr int decimalNotation = 10;
const std::string KEY_ENROLL_ENGINE_UID = "EnrollEngineUid";
const std::string KEY_WAKEUP_ENGINE_BUNDLE_NAME = "WakeupEngineBundleName";
const std::string KEY_WAKEUP_ENGINE_ABILITY_NAME = "WakeupEngineAbilityName";
const std::string KEY_WAKEUP_VESRION = "WakeupVersion";

HistoryInfoMgr::HistoryInfoMgr()
    : ServiceDbHelper("intell_voice_service_manager", "local_intell_voice_history_mgr_storeId")
{
}

void HistoryInfoMgr::SetEnrollEngineUid(int32_t uid)
{
    SetValue(KEY_ENROLL_ENGINE_UID, StringUtil::Int2String(uid));
}

int32_t HistoryInfoMgr::GetEnrollEngineUid()
{
    std::string value = GetValue(KEY_ENROLL_ENGINE_UID);
    return static_cast<int32_t>(strtol(value.c_str(), nullptr, decimalNotation));
}

void HistoryInfoMgr::SetWakeupEngineBundleName(const std::string &bundleName)
{
    SetValue(KEY_WAKEUP_ENGINE_BUNDLE_NAME, bundleName);
}

std::string HistoryInfoMgr::GetWakeupEngineBundleName()
{
    return GetValue(KEY_WAKEUP_ENGINE_BUNDLE_NAME);
}

void HistoryInfoMgr::SetWakeupEngineAbilityName(const std::string &abilityName)
{
    SetValue(KEY_WAKEUP_ENGINE_ABILITY_NAME, abilityName);
}

std::string HistoryInfoMgr::GetWakeupEngineAbilityName()
{
    return GetValue(KEY_WAKEUP_ENGINE_ABILITY_NAME);
}

void HistoryInfoMgr::SetWakeupVesion(const std::string &wakeupVesion)
{
    SetValue(KEY_WAKEUP_VESRION, wakeupVesion);
}

std::string HistoryInfoMgr::GetWakeupVesion()
{
    return GetValue(KEY_WAKEUP_VESRION);
}
}
}