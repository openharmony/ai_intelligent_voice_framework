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
#include "update_engine_controller.h"
#include <fstream>
#include "securec.h"
#include "intell_voice_log.h"

#include "update_adapter_listener.h"
#include "history_info_mgr.h"
#include "time_util.h"
#include "scope_guard.h"
#include "trigger_manager.h"
#include "adapter_callback_service.h"
#include "string_util.h"

#define LOG_TAG "UpdateEngineController"

using namespace OHOS::IntellVoiceTrigger;
using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::AudioStandard;

namespace OHOS {
namespace IntellVoiceEngine {
const std::string INTELL_VOICE_DEFAULT_RES_PATH = "/sys_prod/variant/region_comm/china/etc/intellvoice/wakeup/";
const std::string INTELL_VOICE_CONDICT_PATH_DSP = "/dsp/condict/";
static constexpr uint32_t WAKEUP_VERSION_SPLIT_NUM = 2;
static constexpr uint32_t WAKEUP_VERSION_NUMBERS = 3;
static constexpr int32_t MS_PER_S = 1000;
static constexpr int32_t SILENCE_UPDATE_RETRY_TIMES = 5;

std::atomic<bool> UpdateEngineController::isUpdating_ = false;

UpdateEngineController::UpdateEngineController()
{
}

UpdateEngineController::~UpdateEngineController()
{
}

void UpdateEngineController::GetCurWakeupVesion(std::string &versionNumber)
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

void UpdateEngineController::SaveWakeupVesion()
{
    std::string versionNumber;
    HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();

    GetCurWakeupVesion(versionNumber);
    if (versionNumber.empty()) {
        return;
    }

    historyInfoMgr.SetWakeupVesion(versionNumber);
}

bool UpdateEngineController::IsVersionUpdate()
{
    std::string versionNumberSave;
    std::string versionNumberCur;
    HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();

    versionNumberSave = historyInfoMgr.GetWakeupVesion();
    if (versionNumberSave.empty()) {
        INTELL_VOICE_LOG_ERROR("versionNumberSave is null");
        return false;
    }

    GetCurWakeupVesion(versionNumberCur);
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

void UpdateEngineController::OnTimerEvent(TimerEvent &info)
{
    INTELL_VOICE_LOG_INFO("TimerEvent %{public}d", timerId_);
    OnUpdateRetry();
}

void UpdateEngineController::OnUpdateRetry()
{
    std::lock_guard<std::mutex> lock(updateEngineMutex_);

    if (IsVersionUpdate()) {
        if (CreateUpdateEngine()) {
            INTELL_VOICE_LOG_INFO("retry update, times %{public}d", retryTimes);
            timerId_ = INVALID_ID;
            return;
        }
        updateResult_ = UpdateState::UPDATE_STATE_COMPLETE_FAIL;
    } else {
        updateResult_ = UpdateState::UPDATE_STATE_COMPLETE_SUCCESS;
    }

    INTELL_VOICE_LOG_INFO("retry err, times %{public}d, result %{public}d", retryTimes, updateResult_);
    ReleaseUpdateEngine();
    UpdateCompleteHandler(updateResult_, true);
    ClearRetryState();
    std::thread([this]() { TimerMgr::Stop(); }).detach();
}

bool UpdateEngineController::CreateUpdateEngineUntilTime(int delaySecond)
{
    std::lock_guard<std::mutex> lock(updateEngineMutex_);

    if (GetUpdateState()) {
        INTELL_VOICE_LOG_INFO("update exit");
        return true;
    }

    if (!IsVersionUpdate()) {
        INTELL_VOICE_LOG_INFO("version not update");
        return false;
    }

    delaySecond_ = delaySecond;
    if (CreateUpdateEngine()) {
        INTELL_VOICE_LOG_INFO("create update engine success");
        SetUpdateState(true);
        TimerMgr::Start(nullptr);
        return true;
    }

    return false;
}

void UpdateEngineController::StartUpdateTimer()
{
    if (timerId_ != INVALID_ID) {
        INTELL_VOICE_LOG_INFO("timer already start %d", timerId_);
        return;
    }

    timerId_ = SetTimer(0, delaySecond_ * MS_PER_S * US_PER_MS, 0, this);
    if (timerId_ == INVALID_ID) {
        INTELL_VOICE_LOG_ERROR("timerId %{public}d is invalid", timerId_);
        return;
    }

    retryTimes++;

    INTELL_VOICE_LOG_INFO("start update timer");
}

void UpdateEngineController::StopUpdateTimer()
{
    if (timerId_ == INVALID_ID) {
        INTELL_VOICE_LOG_INFO("timer already stop");
        return;
    }

    KillTimer(timerId_);
    INTELL_VOICE_LOG_INFO("stop update timer");
}

bool UpdateEngineController::IsNeedRetryUpdate()
{
    if (delaySecond_ == 0) {
        INTELL_VOICE_LOG_INFO("not start timer");
        return false;
    }

    if (updateResult_ == UpdateState::UPDATE_STATE_COMPLETE_SUCCESS) {
        INTELL_VOICE_LOG_INFO("update success");
        return false;
    }

    if (retryTimes >= SILENCE_UPDATE_RETRY_TIMES) {
        INTELL_VOICE_LOG_INFO("retry limit");
        return false;
    }

    if (!IsVersionUpdate()) {
        return false;
    }

    return true;
}

void UpdateEngineController::ClearRetryState()
{
    retryTimes = 0;
    delaySecond_ = 0;
    updateResult_ = UpdateState::UPDATE_STATE_DEFAULT;
    timerId_ = INVALID_ID;
    SetUpdateState(false);
}

void UpdateEngineController::OnUpdateComplete(UpdateState result)
{
    std::lock_guard<std::mutex> lock(updateEngineMutex_);
    bool isLast = false;

    updateResult_ = result;
    StopUpdateTimer();

    if (IsNeedRetryUpdate()) {
        StartUpdateTimer();
    } else {
        ClearRetryState();
        isLast = true;
        TimerMgr::Stop();
    }

    ReleaseUpdateEngine();
    UpdateCompleteHandler(result, isLast);
}
}
}