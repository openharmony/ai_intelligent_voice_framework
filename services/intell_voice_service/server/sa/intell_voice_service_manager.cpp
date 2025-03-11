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
#include "intell_voice_service_manager.h"

#include <vector>
#include <fstream>
#include <cstdio>
#include <any>
#include "intell_voice_log.h"
#include "intell_voice_util.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "intell_voice_generic_factory.h"
#include "memory_guard.h"
#include "string_util.h"
#include "json/json.h"
#include "history_info_mgr.h"
#include "ability_manager_client.h"

#define LOG_TAG "IntellVoiceServiceManager"

using namespace OHOS::IntellVoiceTrigger;
using namespace OHOS::IntellVoiceEngine;
using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {
static constexpr int32_t WAIT_AUDIO_HAL_INTERVAL = 500; //500ms
static constexpr uint32_t MAX_TASK_NUM = 2048;
static constexpr uint32_t WAIT_SWICTH_ON_TIME = 2000;  // 2000ms
static constexpr uint32_t WAIT_RECORD_START_TIME = 10000; // 10s
static const std::string STOP_ALL_RECOGNITION = "stop_all_recognition";
static const std::string SHORT_WORD_SWITCH = "short_word_switch";
template<typename T, typename E>
IntellVoiceServiceManager<T, E>::IntellVoiceServiceManager() : TaskExecutor("ServMgrThread", MAX_TASK_NUM)
{
    TaskExecutor::StartThread();
#ifdef ENGINE_ENABLE
    RegisterEngineCallbacks();
#endif
#ifdef TRIGGER_ENABLE
    RegisterTriggerCallbacks();
#endif
#ifdef USE_FFRT
    taskQueue_ = std::make_shared<ffrt::queue>("ServMgrQueue");
#endif
}

template<typename T, typename E>
IntellVoiceServiceManager<T, E>::~IntellVoiceServiceManager()
{
    INTELL_VOICE_LOG_INFO("enter");
#ifdef USE_FFRT
    if (taskQueue_ != nullptr) {
        taskQueue_.reset();
    }
#endif
    TaskExecutor::StopThread();
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::CreateSwitchProvider()
{
    INTELL_VOICE_LOG_INFO("enter");
    std::lock_guard<std::mutex> lock(switchMutex_);
    switchProvider_ = UniquePtrFactory<SwitchProvider>::CreateInstance();
    if (switchProvider_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("switchProvider_ is nullptr");
        return;
    }
    RegisterObserver(WAKEUP_KEY);
    RegisterObserver(WHISPER_KEY);
    RegisterObserver(SHORTWORD_KEY);
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::RegisterObserver(const std::string &switchKey)
{
    switchObserver_[switchKey] = sptr<SwitchObserver>(new (std::nothrow) SwitchObserver());
    if (switchObserver_[switchKey] == nullptr) {
        INTELL_VOICE_LOG_ERROR("switchObserver_ is nullptr");
        return;
    }
    switchObserver_[switchKey]->SetUpdateFunc([this, switchKey]() {
        OnSwitchChange(switchKey);
    });

    switchProvider_->RegisterObserver(switchObserver_[switchKey], switchKey);
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::ReleaseSwitchProvider()
{
    std::lock_guard<std::mutex> lock(switchMutex_);
    if (switchProvider_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("switchProvider_ is nullptr");
        return;
    }

    for (auto it : switchObserver_) {
        if (it.second != nullptr) {
            switchProvider_->UnregisterObserver(it.second, it.first);
        }
    }
    switchProvider_ = nullptr;
}

template<typename T, typename E>
bool IntellVoiceServiceManager<T, E>::StartDetection(int32_t uuid)
{
    if (E::AnyEngineExist({INTELL_VOICE_ENROLL, INTELL_VOICE_UPDATE})) {
        INTELL_VOICE_LOG_INFO("enroll engine or update engine exist, do nothing");
        return false;
    }
    if (!QuerySwitchByUuid(uuid)) {
        INTELL_VOICE_LOG_INFO("switch is off, uuid is %{public}d", uuid);
        return false;
    }

    int32_t ret = T::StartDetection(uuid);
    if (ret == 1) {
        isStarted_[uuid] = true;
        return true;
    }

    if (ret == 0) {
        return AddStartDetectionTask(uuid);
    }

    return false;
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::StopDetection(int32_t uuid)
{
    T::StopDetection(uuid);
}

template<typename T, typename E>
bool IntellVoiceServiceManager<T, E>::QuerySwitchStatus(const std::string &key)
{
    std::lock_guard<std::mutex> lock(switchMutex_);
    if (!IsSwitchKeyValid(key)) {
        INTELL_VOICE_LOG_ERROR("invalid key :%{public}s", key.c_str());
        return false;
    }

    if (switchProvider_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("switchProvider_ is nullptr");
        return false;
    }
    return switchProvider_->QuerySwitchStatus(key);
}

template<typename T, typename E>
bool IntellVoiceServiceManager<T, E>::IsSwitchError(const std::string &key)
{
    std::lock_guard<std::mutex> lock(switchMutex_);
    if (switchProvider_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("switchProvider_ is nullptr");
        return true;
    }
 
    return switchProvider_->IsSwitchError(key);
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::NotifyEvent(const std::string &eventType)
{
    AAFwk::Want want;
    HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();
 
    std::string bundleName = historyInfoMgr.GetStringKVPair(KEY_WAKEUP_ENGINE_BUNDLE_NAME);
    std::string abilityName = historyInfoMgr.GetStringKVPair(KEY_WAKEUP_ENGINE_ABILITY_NAME);
    INTELL_VOICE_LOG_INFO("bundleName:%{public}s, abilityName:%{public}s", bundleName.c_str(), abilityName.c_str());
    if (bundleName.empty() || abilityName.empty()) {
        INTELL_VOICE_LOG_ERROR("bundle name is empty or ability name is empty");
        return;
    }
    want.SetElementName(bundleName, abilityName);
    want.SetParam("serviceName", std::string("intell_voice"));
    want.SetParam("servicePid", getpid());
    want.SetParam("eventType", eventType);
    auto abilityManagerClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityManagerClient == nullptr) {
        INTELL_VOICE_LOG_ERROR("abilityManagerClient is nullptr");
        return;
    }
    abilityManagerClient->StartAbility(want);
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::OnSwitchChange(const std::string &switchKey)
{
    if (switchKey == WAKEUP_KEY) {
        if (QuerySwitchStatus(switchKey)) {
            NoiftySwitchOnToPowerChange();
            HandleSwitchOn(false, VOICE_WAKEUP_MODEL_UUID, false);
        } else {
            HandleSwitchOff(false, VOICE_WAKEUP_MODEL_UUID);
            INTELL_VOICE_LOG_INFO("switch off process finish");
            HandleUnloadIntellVoiceService(false);
        }
    } else if (switchKey == WHISPER_KEY) {
        if (QuerySwitchStatus(switchKey)) {
            NoiftySwitchOnToPowerChange();
            HandleSwitchOn(false, PROXIMAL_WAKEUP_MODEL_UUID, false);
        } else {
            HandleSwitchOff(false, PROXIMAL_WAKEUP_MODEL_UUID);
            HandleUnloadIntellVoiceService(false);
        }
    } else if (switchKey == SHORTWORD_KEY) {
        TaskExecutor::AddSyncTask([this]() -> int32_t {
            INTELL_VOICE_LOG_INFO("short word switch change");
            if (E::AnyEngineExist({INTELL_VOICE_ENROLL, INTELL_VOICE_UPDATE})) {
                INTELL_VOICE_LOG_INFO("enroll engine or update engine exist, do nothing");
                return 0;
            }
            StopDetection(VOICE_WAKEUP_MODEL_UUID);
            StopDetection(PROXIMAL_WAKEUP_MODEL_UUID);
            E::CreateOrResetWakeupEngine();
            SetShortWordStatus();
            StartDetection(VOICE_WAKEUP_MODEL_UUID);
            StartDetection(PROXIMAL_WAKEUP_MODEL_UUID);
            return 0;
        });
    }
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::SetShortWordStatus()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (QuerySwitchStatus(SHORTWORD_KEY)) {
        INTELL_VOICE_LOG_INFO("short_word_switch true");
        T::SetParameter(SHORT_WORD_SWITCH, "1");
    } else {
        INTELL_VOICE_LOG_INFO("short_word_switch false");
        T::SetParameter(SHORT_WORD_SWITCH, "0");
    }
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::ProcBreathModel()
{
    INTELL_VOICE_LOG_INFO("enter");
    std::shared_ptr<uint8_t> buffer = nullptr;
    uint32_t size = 0;
    if (!IntellVoiceUtil::ReadFile(WHISPER_MODEL_PATH, buffer, size)) {
        return;
    }
    std::vector<uint8_t> data(buffer.get(), buffer.get() + size);
    T::UpdateModel(data, PROXIMAL_WAKEUP_MODEL_UUID, TriggerModelType::PROXIMAL_WAKEUP_TYPE);
}

template<typename T, typename E>
int32_t IntellVoiceServiceManager<T, E>::ClearUserData()
{
    INTELL_VOICE_LOG_INFO("enter");
    T::DeleteModel(VOICE_WAKEUP_MODEL_UUID);
    T::DeleteModel(PROXIMAL_WAKEUP_MODEL_UUID);
    return TaskExecutor::AddSyncTask([this]() -> int32_t {
        E::ClearUserDataInner();
        HistoryInfoMgr::GetInstance().DeleteKey({KEY_WAKEUP_ENGINE_BUNDLE_NAME, KEY_WAKEUP_ENGINE_ABILITY_NAME,
            KEY_WAKEUP_VESRION, KEY_LANGUAGE, KEY_AREA, KEY_WAKEUP_PHRASE});
        return UnloadIntellVoiceService();
    });
}

template<typename T, typename E>
bool IntellVoiceServiceManager<T, E>::IsNeedToUnloadService()
{
    if (E::AnyEngineExist({INTELL_VOICE_ENROLL, INTELL_VOICE_UPDATE})) {
        INTELL_VOICE_LOG_INFO("enroll engine or update engine exist, no need to unload service");
        return false;
    }
    if ((QuerySwitchStatus(WAKEUP_KEY)) || (QuerySwitchStatus(WHISPER_KEY))) {
        INTELL_VOICE_LOG_INFO("switch is on, no need to unload service");
        return false;
    }

    return true;
}

template<typename T, typename E>
int32_t IntellVoiceServiceManager<T, E>::UnloadIntellVoiceService()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (!IsNeedToUnloadService()) {
        return 0;
    }

    std::thread([]() {
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityMgr == nullptr) {
            INTELL_VOICE_LOG_ERROR("failed to get systemabilitymanager");
            return;
        }

        int32_t ret = systemAbilityMgr->UnloadSystemAbility(INTELL_VOICE_SERVICE_ID);
        if (ret != 0) {
            INTELL_VOICE_LOG_ERROR("failed to unload intellvoice service, ret: %{public}d", ret);
            return;
        }
        INTELL_VOICE_LOG_INFO("success to notify samgr to unload intell voice service");
    }).detach();

    return 0;
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::HandleUnloadIntellVoiceService(bool isAsync)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (isAsync) {
        TaskExecutor::AddAsyncTask([this]() { UnloadIntellVoiceService(); });
    } else {
        TaskExecutor::AddSyncTask([this]() -> int32_t { return UnloadIntellVoiceService(); });
    }
}

template<typename T, typename E>
bool IntellVoiceServiceManager<T, E>::HandleOnIdle()
{
    return TaskExecutor::AddSyncTask([this]() -> bool { return IsNeedToUnloadService(); });
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::NoiftySwitchOnToPowerChange()
{
    {
        std::unique_lock<std::mutex> lock(powerModeChangeMutex_);
        if (!notifyPowerModeChange_) {
            INTELL_VOICE_LOG_INFO("no need to notify");
            return;
        }
    }

    powerModeChangeCv_.notify_all();
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::HandlePowerSaveModeChange()
{
    if ((QuerySwitchStatus(WAKEUP_KEY)) || (QuerySwitchStatus(WHISPER_KEY))) {
        INTELL_VOICE_LOG_INFO("switch is on, no need to process");
        return;
    }

    std::thread([&]() {
        std::unique_lock<std::mutex> lock(powerModeChangeMutex_);
        if ((QuerySwitchStatus(WAKEUP_KEY)) || (QuerySwitchStatus(WHISPER_KEY))) {
            INTELL_VOICE_LOG_INFO("switch is on, no need to process");
            return;
        }
        notifyPowerModeChange_ = true;
        if (powerModeChangeCv_.wait_for(lock, std::chrono::milliseconds(WAIT_SWICTH_ON_TIME),
            [this] { return ((this->QuerySwitchStatus(WAKEUP_KEY)) || (this->QuerySwitchStatus(WHISPER_KEY))); })) {
            INTELL_VOICE_LOG_INFO("switch is on, do nothing");
            notifyPowerModeChange_ = false;
            return;
        }
        INTELL_VOICE_LOG_WARN("wait time out, switch is off, need to unload service");
        notifyPowerModeChange_ = false;
        HandleUnloadIntellVoiceService(true);
    }).detach();
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::OnTriggerConnectServiceStart()
{
    INTELL_VOICE_LOG_INFO("enter");
    TaskExecutor::AddAsyncTask([this]() { SwitchOnProc(VOICE_WAKEUP_MODEL_UUID, false); },
        "IntellVoiceServiceManager::OnTriggerConnectServiceStart::start wakeup", false);
    TaskExecutor::AddAsyncTask([this]() { SwitchOnProc(PROXIMAL_WAKEUP_MODEL_UUID, false); },
        "IntellVoiceServiceManager::OnTriggerConnectServiceStart::start proximal", false);
}

template<typename T, typename E>
sptr<IIntellVoiceEngine> IntellVoiceServiceManager<T, E>::HandleCreateEngine(IntellVoiceEngineType type)
{
    return TaskExecutor::AddSyncTask([this, type]() -> sptr<IIntellVoiceEngine> {
            if (type == INTELL_VOICE_ENROLL) {
                StopDetection(VOICE_WAKEUP_MODEL_UUID);
                StopDetection(PROXIMAL_WAKEUP_MODEL_UUID);
            }
            return E::CreateEngine(type);
    });
}

template<typename T, typename E>
int32_t IntellVoiceServiceManager<T, E>::HandleReleaseEngine(IntellVoiceEngineType type)
{
    return TaskExecutor::AddSyncTask([this, type]() -> int32_t {return ReleaseEngine(type); });
}

template<typename T, typename E>
int32_t IntellVoiceServiceManager<T, E>::ReleaseEngine(IntellVoiceEngineType type)
{
    INTELL_VOICE_LOG_INFO("enter, type:%{public}d", type);
    auto ret = E::ReleaseEngineInner(type);
    if (ret != 0) {
        return ret;
    }

    if (type != INTELL_VOICE_ENROLL) {
        return 0;
    }

    if (E::GetEnrollResult(type)) {
        SwitchOnProc(VOICE_WAKEUP_MODEL_UUID, true);
        SwitchOnProc(PROXIMAL_WAKEUP_MODEL_UUID, true);
        SwitchOffProc(PROXIMAL_WAKEUP_MODEL_UUID);
    } else {
        SwitchOnProc(VOICE_WAKEUP_MODEL_UUID, true);
        SwitchOffProc(VOICE_WAKEUP_MODEL_UUID);
        SwitchOnProc(PROXIMAL_WAKEUP_MODEL_UUID, true);
        SwitchOffProc(PROXIMAL_WAKEUP_MODEL_UUID);
        UnloadIntellVoiceService();
    }

    return 0;
}

template<typename T, typename E>
int32_t IntellVoiceServiceManager<T, E>::SwitchOnProc(int32_t uuid, bool needUpdateAdapter)
{
    INTELL_VOICE_LOG_INFO("enter, uuid:%{public}d", uuid);
    if (E::AnyEngineExist({INTELL_VOICE_ENROLL, INTELL_VOICE_UPDATE})) {
        INTELL_VOICE_LOG_INFO("enroll engine or update engine exist, do nothing");
        return 0;
    }
    if (!QuerySwitchByUuid(uuid)) {
        INTELL_VOICE_LOG_INFO("switch is off, do nothing, uuid is %{public}d", uuid);
        return 0;
    }

    CreateAndStartServiceObject(uuid, needUpdateAdapter);
    INTELL_VOICE_LOG_INFO("exit");
    return 0;
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::CreateAndStartServiceObject(int32_t uuid, bool needResetAdapter)
{
    if (!T::IsModelExist(uuid)) {
        INTELL_VOICE_LOG_INFO("no model");
        return;
    }

    if (!QuerySwitchByUuid(uuid)) {
        INTELL_VOICE_LOG_INFO("switch is off, uuid is %{public}d", uuid);
        return;
    }

#ifndef ONLY_FIRST_STAGE
        INTELL_VOICE_LOG_INFO("is not single wakeup level");
        if (!needResetAdapter) {
            E::CreateEngine(INTELL_VOICE_WAKEUP);
        } else {
            E::CreateOrResetWakeupEngine();
        }
        T::CreateDetector(uuid, [this, uuid]() { OnDetected(uuid); });
#else
        T::CreateDetector(uuid, [this, uuid]() { OnSingleLevelDetected(); });
#endif

    if (uuid == VOICE_WAKEUP_MODEL_UUID) {
        SetShortWordStatus();
    }

    if (!StartDetection(uuid)) {
        INTELL_VOICE_LOG_ERROR("failed to start detection");
    }
}

template<typename T, typename E>
int32_t IntellVoiceServiceManager<T, E>::SwitchOffProc(int32_t uuid)
{
    INTELL_VOICE_LOG_INFO("enter, uuid:%{public}d", uuid);
    if (E::AnyEngineExist({INTELL_VOICE_ENROLL, INTELL_VOICE_UPDATE})) {
        INTELL_VOICE_LOG_INFO("enroll engine or update engine exist, do nothing");
        return 0;
    }
    if (QuerySwitchByUuid(uuid)) {
        INTELL_VOICE_LOG_INFO("switch is on, do nothing, uuid is %{public}d", uuid);
        return 0;
    }

    DelStartDetectionTask(uuid);
    ReleaseServiceObject(uuid);
    INTELL_VOICE_LOG_INFO("exit, uuid:%{public}d", uuid);
    return 0;
}

template<typename T, typename E>
bool IntellVoiceServiceManager<T, E>::AddStartDetectionTask(int32_t uuid)
{
#ifdef USE_FFRT
    INTELL_VOICE_LOG_INFO("enter");
    if (taskQueue_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("task queue is nullptr");
        return false;
    }

    if ((taskHandle_.count(uuid) != 0) && (taskHandle_[uuid] != nullptr)) {
        INTELL_VOICE_LOG_INFO("task handle is exist, uuid is %{public}d", uuid);
        return true;
    }

    taskHandle_[uuid] = taskQueue_->submit_h(std::bind([uuid, this]() {
        if (!QuerySwitchByUuid(uuid)) {
            INTELL_VOICE_LOG_INFO("switch is off, uuid is %{public}d", uuid);
            return;
        }

        INTELL_VOICE_LOG_INFO("begin to wait");
        while (T::GetParameter("audio_hal_status") != "true") {
            ffrt::this_task::sleep_for(std::chrono::milliseconds(WAIT_AUDIO_HAL_INTERVAL));
            if (!QuerySwitchByUuid(uuid)) {
                INTELL_VOICE_LOG_INFO("switch is off, uuid is %{public}d", uuid);
                return;
            }
        }
        INTELL_VOICE_LOG_INFO("end to wait");
        TaskExecutor::AddAsyncTask([uuid, this]() {
            if (isStarted_.count(uuid) != 0 && isStarted_[uuid]) {
                INTELL_VOICE_LOG_INFO("already start, uuid is %{public}d", uuid);
                return;
            }
            StartDetection(uuid);
        });
    }));
#endif
    return true;
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::DelStartDetectionTask(int32_t uuid)
{
#ifdef USE_FFRT
    INTELL_VOICE_LOG_INFO("enter");
    if (taskQueue_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("task queue is nullptr");
        return;
    }

    if ((taskHandle_.count(uuid) != 0) && (taskHandle_[uuid] != nullptr)) {
        taskQueue_->cancel(taskHandle_[uuid]);
        taskHandle_.erase(uuid);
        INTELL_VOICE_LOG_INFO("task is canceled");
    }
#endif

    if (isStarted_.count(uuid)) {
        isStarted_.erase(uuid);
    }
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::ReleaseServiceObject(int32_t uuid)
{
    T::ReleaseTriggerDetector(uuid);
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::HandleSilenceUpdate()
{
    TaskExecutor::AddAsyncTask([this]() {
        E::SilenceUpdate();
    });
}

template<typename T, typename E>
int32_t IntellVoiceServiceManager<T, E>::HandleCloneUpdate(const std::string &wakeupInfo,
    const sptr<IRemoteObject> &object)
{
    return TaskExecutor::AddSyncTask([this, wakeupInfo, object = std::move(object)]() -> int32_t {
        return E::CloneUpdate(wakeupInfo, object);
    });
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::StopWakeupSource()
{
    INTELL_VOICE_LOG_INFO("enter, stop all recognition");
    TaskExecutor::AddAsyncTask([this]() {
        T::SetParameter(STOP_ALL_RECOGNITION, "true");
        }, "IntellVoiceServiceManager::StopWakeupSource", false);
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::HandleCloseWakeupSource(bool isNeedStop)
{
    INTELL_VOICE_LOG_INFO("enter, isNeedStop:%{public}d", isNeedStop);
    TaskExecutor::AddAsyncTask([this, isNeedStop]() {
        if (isNeedStop) {
            T::SetParameter(STOP_ALL_RECOGNITION, "true");
        }
        StartDetection(VOICE_WAKEUP_MODEL_UUID);
        StartDetection(PROXIMAL_WAKEUP_MODEL_UUID);
        }, "IntellVoiceServiceManager::HandleCloseWakeupSource", false);
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::HandleServiceStop()
{
    TaskExecutor::AddSyncTask([this]() -> int32_t {
        return E::ServiceStopProc();
    });
    if (IsSwitchError(WAKEUP_KEY)) {
        INTELL_VOICE_LOG_WARN("db is abnormal, can not find wakeup switch, notify db error");
        NotifyEvent("db_error");
    }
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::HandleHeadsetHostDie()
{
    TaskExecutor::AddSyncTask([this]() -> int32_t {
        E::HeadsetHostDie();
        return 0;
    });
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::HandleClearWakeupEngineCb()
{
    TaskExecutor::AddSyncTask([this]() -> int32_t {
        E::ClearWakeupEngineCb();
        return 0;
    });
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::HandleSwitchOff(bool isAsync, int32_t uuid)
{
    INTELL_VOICE_LOG_INFO("enter, isAsync:%{public}d, uuid:%{public}d", isAsync, uuid);
    if (!isAsync) {
        TaskExecutor::AddSyncTask([this, uuid]() -> int32_t { return SwitchOffProc(uuid); });
    } else {
        TaskExecutor::AddAsyncTask([this, uuid]() { SwitchOffProc(uuid); });
    }
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::HandleSwitchOn(bool isAsync, int32_t uuid, bool needUpdateAdapter)
{
    INTELL_VOICE_LOG_INFO("enter, isAsync:%{public}d, uuid:%{public}d, needUpdateAdapter:%{public}d",
        isAsync, uuid, needUpdateAdapter);
    if (isAsync) {
        TaskExecutor::AddAsyncTask([this, uuid, needUpdateAdapter]() { SwitchOnProc(uuid, needUpdateAdapter); });
    } else {
        TaskExecutor::AddSyncTask([this, uuid, needUpdateAdapter]() -> int32_t {
            return SwitchOnProc(uuid, needUpdateAdapter);
        });
    }
    INTELL_VOICE_LOG_INFO("exit");
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::HandleUpdateComplete(int32_t result, const std::string &param)
{
    TaskExecutor::AddAsyncTask([this, result, param]() {
        if (E::IsNeedUpdateComplete(result, param)) {
            SwitchOnProc(VOICE_WAKEUP_MODEL_UUID, true);
            SwitchOnProc(PROXIMAL_WAKEUP_MODEL_UUID, true);
            UnloadIntellVoiceService();
        }
        }, "IntellVoiceServiceManager::HandleUpdateComplete", false);
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::HandleUpdateRetry()
{
    TaskExecutor::AddAsyncTask([this]() {
        if (E::IsNeedUpdateRetry()) {
            SwitchOnProc(VOICE_WAKEUP_MODEL_UUID, true);
            SwitchOnProc(PROXIMAL_WAKEUP_MODEL_UUID, true);
            UnloadIntellVoiceService();
        }
        }, "IntellVoiceServiceManager::HandleUpdateRetry", false);
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::OnDetected(int32_t uuid)
{
    TaskExecutor::AddAsyncTask([uuid, this]() {
        if (!E::IsEngineExist(INTELL_VOICE_WAKEUP)) {
            INTELL_VOICE_LOG_WARN("wakeup engine is nullptr");
            HandleCloseWakeupSource(true);
            return;
        }
        E::EngineOnDetected(uuid);
        }, "IntellVoiceServiceManager::OnDetected", false);
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::OnServiceStart(std::map<int32_t, std::function<void(bool)>> &saChangeFuncMap)
{
    saChangeFuncMap[TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID] = [this](bool isAdded) {
        T::OnTelephonyStateRegistryServiceChange(isAdded);
    };
    saChangeFuncMap[AUDIO_DISTRIBUTED_SERVICE_ID] = [this](bool isAdded) {
        T::OnAudioDistributedServiceChange(isAdded);
    };
    saChangeFuncMap[AUDIO_POLICY_SERVICE_ID] = [this](bool isAdded) {
        T::OnAudioPolicyServiceChange(isAdded);
    };
    saChangeFuncMap[POWER_MANAGER_SERVICE_ID] = [this](bool isAdded) {
        T::OnPowerManagerServiceChange(isAdded);
    };
    T::OnServiceStart();
    E::OnServiceStart();
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::OnServiceStop()
{
    T::OnServiceStop();
    E::OnServiceStop();
}

template<typename T, typename E>
std::string IntellVoiceServiceManager<T, E>::TriggerGetParameter(const std::string &key)
{
    return T::GetParameter(key);
}

template<typename T, typename E>
int32_t IntellVoiceServiceManager<T, E>::TriggerSetParameter(const std::string &key, const std::string &value)
{
    return T::SetParameter(key, value);
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::TriggerMgrUpdateModel(std::vector<uint8_t> buffer, int32_t uuid,
    TriggerModelType type)
{
    T::UpdateModel(buffer, uuid, type);
}

template<typename T, typename E>
bool IntellVoiceServiceManager<T, E>::RegisterProxyDeathRecipient(IntellVoiceEngineType type,
    const sptr<IRemoteObject> &object)
{
    return E::RegisterProxyDeathRecipient(type, object);
}

template<typename T, typename E>
bool IntellVoiceServiceManager<T, E>::DeregisterProxyDeathRecipient(IntellVoiceEngineType type)
{
    return E::DeregisterProxyDeathRecipient(type);
}

template<typename T, typename E>
int32_t IntellVoiceServiceManager<T, E>::GetUploadFiles(int numMax, std::vector<UploadFilesFromHdi> &files)
{
    return E::GetUploadFiles(numMax, files);
}

template<typename T, typename E>
int32_t IntellVoiceServiceManager<T, E>::EngineSetParameter(const std::string &keyValueList)
{
    INTELL_VOICE_LOG_INFO("enter");
    HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();
    std::map<std::string, std::string> kvpairs;
    IntellVoiceUtil::SplitStringToKVPair(keyValueList, kvpairs);
    for (auto it : kvpairs) {
        if (it.first == std::string("Sensibility")) {
            INTELL_VOICE_LOG_INFO("set Sensibility:%{public}s", it.second.c_str());
            std::string sensibility = it.second;
            historyInfoMgr.SetStringKVPair(KEY_SENSIBILITY, sensibility);
            TaskExecutor::AddSyncTask([this, sensibility]() -> int32_t {
                E::SetDspSensibility(sensibility);
                return E::SetParameter(sensibility);
            });
        } else if (it.first == std::string("wakeup_bundle_name")) {
            INTELL_VOICE_LOG_INFO("set wakeup bundle name:%{public}s", it.second.c_str());
            historyInfoMgr.SetStringKVPair(KEY_WAKEUP_ENGINE_BUNDLE_NAME, it.second);
        } else if (it.first == std::string("wakeup_ability_name")) {
            INTELL_VOICE_LOG_INFO("set wakeup ability name:%{public}s", it.second.c_str());
            historyInfoMgr.SetStringKVPair(KEY_WAKEUP_ENGINE_ABILITY_NAME, it.second);
#ifdef ONLY_FIRST_STAGE
        } else if (it.first == std::string("record_start")) {
            ResetSingleLevelWakeup(it.second);
#endif
        } else {
            INTELL_VOICE_LOG_INFO("no need to process, key:%{public}s", it.first.c_str());
        }
    }
    return 0;
}

template<typename T, typename E>
std::string IntellVoiceServiceManager<T, E>::EngineGetParameter(const std::string &key)
{
    return E::GetParameter(key);
}

template<typename T, typename E>
int32_t IntellVoiceServiceManager<T, E>::GetWakeupSourceFilesList(std::vector<std::string>& cloneFiles)
{
    return E::GetWakeupSourceFilesList(cloneFiles);
}

template<typename T, typename E>
int32_t IntellVoiceServiceManager<T, E>::GetWakeupSourceFile(const std::string &filePath,
    std::vector<uint8_t> &buffer)
{
    return E::GetWakeupSourceFile(filePath, buffer);
}

template<typename T, typename E>
int32_t IntellVoiceServiceManager<T, E>::SendWakeupFile(const std::string &filePath,
    const std::vector<uint8_t> &buffer)
{
    return E::SendWakeupFile(filePath, buffer);
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::SetScreenOff(bool value)
{
    return E::SetScreenOff(value);
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::OnSingleLevelDetected()
{
    INTELL_VOICE_LOG_INFO("single level detected");
    recordStart_ = -1;
    StopWakeupSource();
    NotifyEvent("single_level_event");
    HandleRecordStartInfoChange();
    return;
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::ProcSingleLevelModel()
{
#ifdef ONLY_FIRST_STAGE
    INTELL_VOICE_LOG_INFO("enter");
    std::shared_ptr<uint8_t> buffer = nullptr;
    uint32_t size = 0;
    if (!IntellVoiceUtil::ReadFile(SINGLE_LEVEL_MODEL_PATH, buffer, size)) {
        INTELL_VOICE_LOG_ERROR("read model failed!");
        return;
    }
    std::vector<uint8_t> data(buffer.get(), buffer.get() + size);
    T::UpdateModel(data, VOICE_WAKEUP_MODEL_UUID, TriggerModelType::VOICE_WAKEUP_TYPE);
#endif
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::ResetSingleLevelWakeup(const std::string &value)
{
    INTELL_VOICE_LOG_INFO("record_start:%{public}s", value.c_str());
    if (value != std::string("true") && value != std::string("false")) {
        return;
    }

    recordStart_ = (value == std::string("true")) ? 1 : 0;
    NoiftyRecordStartInfoChange();
    if (recordStart_ == 0) {
        HandleCloseWakeupSource(true);
    }
}

template<typename T, typename E>
bool IntellVoiceServiceManager<T, E>::HasReceviedRecordStartMsg()
{
    return (recordStart_ == 1 || recordStart_ == 0);
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::NoiftyRecordStartInfoChange()
{
    {
        std::unique_lock<std::mutex> lock(recordStartInfoChangeMutex_);
        if (!notifyRecordStartInfoChange_) {
            INTELL_VOICE_LOG_INFO("no need to notify");
            return;
        }
    }

    recordStartInfoChangeCv_.notify_all();
}

template<typename T, typename E>
void IntellVoiceServiceManager<T, E>::HandleRecordStartInfoChange()
{
    if (HasReceviedRecordStartMsg()) {
        INTELL_VOICE_LOG_INFO("record start info has received, no need to process");
        return;
    }

    std::thread([&]() {
        std::unique_lock<std::mutex> lock(recordStartInfoChangeMutex_);
        if (HasReceviedRecordStartMsg()) {
            INTELL_VOICE_LOG_INFO("record start info has received, no need to process");
            return;
        }
        notifyRecordStartInfoChange_ = true;
        if (recordStartInfoChangeCv_.wait_for(lock, std::chrono::milliseconds(WAIT_RECORD_START_TIME),
            [this] { return HasReceviedRecordStartMsg(); })) {
            INTELL_VOICE_LOG_INFO("record start info has received, do nothing");
            notifyRecordStartInfoChange_ = false;
            return;
        }
        INTELL_VOICE_LOG_WARN("wait time out, need to reset wakeup");
        notifyRecordStartInfoChange_ = false;
        HandleCloseWakeupSource(true);
    }).detach();
}

template class IntellVoiceServiceManager<TriggerManagerType, EngineManagerType>;
}  // namespace IntellVoiceEngine
} // namespace OHOS