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
#include "intell_voice_log.h"
#include "intell_voice_util.h"
#include "engine_factory.h"
#include "wakeup_engine.h"
#include "trigger_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "intell_voice_generic_factory.h"
#include "trigger_detector_callback.h"
#include "memory_guard.h"
#include "iproxy_broker.h"
#include "engine_host_manager.h"
#include "string_util.h"
#include "clone_update_strategy.h"
#include "silence_update_strategy.h"
#include "update_engine_utils.h"

#define LOG_TAG "IntellVoiceServiceManager"

using namespace OHOS::IntellVoiceTrigger;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;

namespace OHOS {
namespace IntellVoiceEngine {
static constexpr int32_t MAX_ATTEMPT_CNT = 10;
static constexpr uint32_t MAX_TASK_NUM = 200;
static const std::string SERVICE_MANAGER_THREAD_NAME = "ServMgrThread";
static const std::string WHISPER_MODEL_PATH =
    "/sys_prod/variant/region_comm/china/etc/intellvoice/wakeup/dsp/whisper_wakeup_dsp_config";

std::atomic<bool> IntellVoiceServiceManager::enrollResult_[ENGINE_TYPE_BUT] = {false, false, false};

std::unique_ptr<IntellVoiceServiceManager> IntellVoiceServiceManager::g_intellVoiceServiceMgr =
    std::unique_ptr<IntellVoiceServiceManager>(new (std::nothrow) IntellVoiceServiceManager());

IntellVoiceServiceManager::IntellVoiceServiceManager() : TaskExecutor(SERVICE_MANAGER_THREAD_NAME, MAX_TASK_NUM)
{
    TaskExecutor::StartThread();
}

IntellVoiceServiceManager::~IntellVoiceServiceManager()
{
    engines_.clear();
    TaskExecutor::StopThread();
}

std::unique_ptr<IntellVoiceServiceManager> &IntellVoiceServiceManager::GetInstance()
{
    return g_intellVoiceServiceMgr;
}

sptr<IIntellVoiceEngine> IntellVoiceServiceManager::CreateEngine(IntellVoiceEngineType type, const std::string &param)
{
    INTELL_VOICE_LOG_INFO("enter, type:%{public}d", type);
    if (type == INTELL_VOICE_ENROLL) {
        StopDetection(VOICE_WAKEUP_MODEL_UUID);
        StopDetection(PROXIMAL_WAKEUP_MODEL_UUID);
    }

    SetEnrollResult(type, false);
    if (ApplyArbitration(type, engines_) != ARBITRATION_OK) {
        INTELL_VOICE_LOG_ERROR("policy manager reject create engine, type:%{public}d", type);
        return nullptr;
    }

    if (type != INTELL_VOICE_WAKEUP) {
        auto engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
        if (engine != nullptr) {
            engine->ReleaseAdapter();
        }
    }

    return CreateEngineInner(type, param);
}

sptr<IIntellVoiceEngine> IntellVoiceServiceManager::CreateEngineInner(IntellVoiceEngineType type,
    const std::string &param)
{
    INTELL_VOICE_LOG_INFO("create engine enter, type: %{public}d", type);
    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    sptr<EngineBase> engine = GetEngine(type, engines_);
    if (engine != nullptr) {
        return engine;
    }

    engine = EngineFactory::CreateEngineInst(type, param);
    if (engine == nullptr) {
        INTELL_VOICE_LOG_ERROR("create engine failed, type:%{public}d", type);
        return nullptr;
    }

    SetImproveParam(engine);
    engines_[type] = engine;
    INTELL_VOICE_LOG_INFO("create engine ok");
    return engine;
}

int32_t IntellVoiceServiceManager::ReleaseEngine(IntellVoiceEngineType type)
{
    INTELL_VOICE_LOG_INFO("enter, type:%{public}d", type);
    auto ret = ReleaseEngineInner(type);
    if (ret != 0) {
        return ret;
    }

    if (type != INTELL_VOICE_ENROLL) {
        return 0;
    }

    if (GetEnrollResult(type)) {
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

int32_t IntellVoiceServiceManager::ReleaseEngineInner(IntellVoiceEngineType type)
{
    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    auto it = engines_.find(type);
    if (it == engines_.end()) {
        INTELL_VOICE_LOG_WARN("there is no engine(%{public}d) in list", type);
        return 0;
    }

    if (it->second != nullptr) {
        it->second->Detach();
        it->second = nullptr;
    }

    engines_.erase(type);
    return 0;
}

bool IntellVoiceServiceManager::CreateOrResetWakeupEngine()
{
    auto engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
    if (engine != nullptr) {
        INTELL_VOICE_LOG_INFO("wakeup engine is existed");
        engine->ReleaseAdapter();
        if (!engine->ResetAdapter()) {
            INTELL_VOICE_LOG_ERROR("failed to reset adapter");
            return false;
        }
        SetImproveParam(engine);
    } else {
        if (CreateEngineInner(INTELL_VOICE_WAKEUP) == nullptr) {
            INTELL_VOICE_LOG_ERROR("failed to create wakeup engine");
            return false;
        }
    }
    return true;
}

void IntellVoiceServiceManager::CreateSwitchProvider()
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
    RegisterObserver(IMPROVE_KEY);
    RegisterObserver(SHORTWORD_KEY);
}

void IntellVoiceServiceManager::RegisterObserver(const std::string &switchKey)
{
    switchObserver_[switchKey] = sptr<SwitchObserver>(new (std::nothrow) SwitchObserver());
    if (switchObserver_[switchKey] == nullptr) {
        INTELL_VOICE_LOG_ERROR("switchObserver_ is nullptr");
        return;
    }
    switchObserver_[switchKey]->SetUpdateFunc([switchKey]() {
        const auto &manager = IntellVoiceServiceManager::GetInstance();
        if (manager != nullptr) {
            manager->OnSwitchChange(switchKey);
        }
    });

    switchProvider_->RegisterObserver(switchObserver_[switchKey], switchKey);
}

void IntellVoiceServiceManager::ReleaseSwitchProvider()
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

void IntellVoiceServiceManager::CreateDetector(int32_t uuid)
{
    std::lock_guard<std::mutex> lock(detectorMutex_);
    if (!QuerySwitchByUuid(uuid)) {
        INTELL_VOICE_LOG_INFO("switch is off, uuid is %{public}d", uuid);
        return;
    }

    if (detector_.count(uuid) != 0 && detector_[uuid] != nullptr) {
        INTELL_VOICE_LOG_INFO("detector is already existed, no need to create, uuid:%{public}d", uuid);
        return;
    }

    auto cb = std::make_shared<TriggerDetectorCallback>([&, uuid]() { OnDetected(uuid); });
    if (cb == nullptr) {
        INTELL_VOICE_LOG_ERROR("cb is nullptr");
        return;
    }

    auto triggerMgr = TriggerManager::GetInstance();
    if (triggerMgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger manager is nullptr");
        return;
    }

    auto detector = triggerMgr->CreateTriggerDetector(uuid, cb);
    if (detector == nullptr) {
        INTELL_VOICE_LOG_ERROR("detector is nullptr");
        return;
    }

    detector_[uuid] = detector;
}

void IntellVoiceServiceManager::StartDetection(int32_t uuid)
{
    if ((IsEngineExist(INTELL_VOICE_ENROLL)) || (IsEngineExist(INTELL_VOICE_UPDATE))) {
        INTELL_VOICE_LOG_INFO("enroll engine or update engine exist, do nothing");
        return;
    }

    for (uint32_t cnt = 1; cnt <= MAX_ATTEMPT_CNT; ++cnt) {
        {
            std::lock_guard<std::mutex> lock(detectorMutex_);
            if ((detector_.count(uuid) == 0) || (detector_[uuid] == nullptr)) {
                INTELL_VOICE_LOG_INFO("detector is not existed, uuid:%{public}d", uuid);
                return;
            }

            if (!QuerySwitchByUuid(uuid)) {
                INTELL_VOICE_LOG_INFO("switch is off, uuid is %{public}d", uuid);
                return;
            }

            auto triggerMgr = TriggerManager::GetInstance();
            if (triggerMgr == nullptr) {
                INTELL_VOICE_LOG_ERROR("trigger manager is nullptr");
                return;
            }

            if (triggerMgr->GetParameter("audio_hal_status") == "true") {
                INTELL_VOICE_LOG_INFO("audio hal is ready");
                detector_[uuid]->StartRecognition();
                return;
            }
        }
        INTELL_VOICE_LOG_INFO("begin to wait");
        std::this_thread::sleep_for(std::chrono::seconds(cnt));
        INTELL_VOICE_LOG_INFO("end to wait");
    }
    INTELL_VOICE_LOG_ERROR("failed to start recognition");
}

void IntellVoiceServiceManager::StopDetection(int32_t uuid)
{
    std::lock_guard<std::mutex> lock(detectorMutex_);
    if ((detector_.count(uuid) == 0) || (detector_[uuid] == nullptr)) {
        INTELL_VOICE_LOG_INFO("detector is not existed, uuid:%{public}d", uuid);
        return;
    }
    detector_[uuid]->StopRecognition();
}

void IntellVoiceServiceManager::OnDetected(int32_t uuid)
{
    TaskExecutor::AddAsyncTask([uuid, this]() {
        auto engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
        if (engine == nullptr) {
            return;
        }
        engine->OnDetected(uuid);
    });
}

void IntellVoiceServiceManager::CreateAndStartServiceObject(int32_t uuid, bool needResetAdapter)
{
    auto triggerMgr = TriggerManager::GetInstance();
    if (triggerMgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger manager is nullptr");
        return;
    }
    auto model = triggerMgr->GetModel(uuid);
    if (model == nullptr) {
        INTELL_VOICE_LOG_INFO("no model");
        return;
    }

    if (!QuerySwitchByUuid(uuid)) {
        INTELL_VOICE_LOG_INFO("switch is off, uuid is %{public}d", uuid);
        return;
    }

    if (!needResetAdapter) {
        CreateEngine(INTELL_VOICE_WAKEUP);
    } else {
        CreateOrResetWakeupEngine();
    }
    CreateDetector(uuid);
    StartDetection(uuid);
}

void IntellVoiceServiceManager::ReleaseServiceObject(int32_t uuid)
{
    std::lock_guard<std::mutex> lock(detectorMutex_);
    auto it = detector_.find(uuid);
    if (it != detector_.end()) {
        if (it->second != nullptr) {
            it->second->UnloadTriggerModel();
        }
        detector_.erase(it);
    }

    auto triggerMgr = TriggerManager::GetInstance();
    if (triggerMgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger manager is nullptr");
        return;
    }
    triggerMgr->ReleaseTriggerDetector(uuid);
}

int32_t IntellVoiceServiceManager::ServiceStopProc()
{
    sptr<EngineBase> wakeupEngine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
    if (wakeupEngine == nullptr) {
        INTELL_VOICE_LOG_INFO("wakeup engine is not existed");
        return -1;
    }
    wakeupEngine->Detach();
    return 0;
}

bool IntellVoiceServiceManager::QuerySwitchStatus(const std::string &key)
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

void IntellVoiceServiceManager::OnSwitchChange(const std::string &switchKey)
{
    if (switchKey == WAKEUP_KEY) {
        if (QuerySwitchStatus(switchKey)) {
            HandleSwitchOn(false, VOICE_WAKEUP_MODEL_UUID, false);
        } else {
            HandleSwitchOff(false, VOICE_WAKEUP_MODEL_UUID);
            INTELL_VOICE_LOG_INFO("switch off process finish");
            HandleUnloadIntellVoiceService(false);
        }
    } else if (switchKey == WHISPER_KEY) {
        if (QuerySwitchStatus(switchKey)) {
            HandleSwitchOn(false, PROXIMAL_WAKEUP_MODEL_UUID, false);
        } else {
            HandleSwitchOff(false, PROXIMAL_WAKEUP_MODEL_UUID);
            HandleUnloadIntellVoiceService(false);
        }
    } else if (switchKey == IMPROVE_KEY) {
        TaskExecutor::AddSyncTask([this]() {
            auto engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
            if (engine != nullptr) {
                SetImproveParam(engine);
            }
            return 0;
        });
    } else if (switchKey == SHORTWORD_KEY) {
        TaskExecutor::AddSyncTask([this]() {
            INTELL_VOICE_LOG_INFO("short word switch change");
            StopDetection(VOICE_WAKEUP_MODEL_UUID);
            StopDetection(PROXIMAL_WAKEUP_MODEL_UUID);
            CreateOrResetWakeupEngine();
            StartDetection(VOICE_WAKEUP_MODEL_UUID);
            StartDetection(PROXIMAL_WAKEUP_MODEL_UUID);
        });
    }
}

bool IntellVoiceServiceManager::RegisterProxyDeathRecipient(IntellVoiceEngineType type,
    const sptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(deathMutex_);
    INTELL_VOICE_LOG_INFO("enter, type:%{public}d", type);
    deathRecipientObj_[type] = object;
    if (type == INTELL_VOICE_ENROLL) {
        proxyDeathRecipient_[type] = new (std::nothrow) IntellVoiceDeathRecipient([&]() {
            INTELL_VOICE_LOG_INFO("receive enroll proxy death recipient, release enroll engine");
            HandleReleaseEngine(INTELL_VOICE_ENROLL);
        });
        if (proxyDeathRecipient_[type] == nullptr) {
            INTELL_VOICE_LOG_ERROR("create death recipient failed");
            return false;
        }
    } else if (type == INTELL_VOICE_WAKEUP) {
        proxyDeathRecipient_[type] = new (std::nothrow) IntellVoiceDeathRecipient([&]() {
            INTELL_VOICE_LOG_INFO("receive wakeup proxy death recipient, clear wakeup engine callback");
            TaskExecutor::AddSyncTask([&]() -> int32_t {
                sptr<EngineBase> engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
                if (engine != nullptr) {
                    INTELL_VOICE_LOG_INFO("clear wakeup engine callback");
                    engine->SetCallback(nullptr);
                }
                return 0;
            });
        });
        if (proxyDeathRecipient_[type] == nullptr) {
            INTELL_VOICE_LOG_ERROR("create death recipient failed");
            return false;
        }
    } else {
        INTELL_VOICE_LOG_ERROR("invalid type:%{public}d", type);
        return false;
    }

    return deathRecipientObj_[type]->AddDeathRecipient(proxyDeathRecipient_[type]);
}

bool IntellVoiceServiceManager::DeregisterProxyDeathRecipient(IntellVoiceEngineType type)
{
    std::lock_guard<std::mutex> lock(deathMutex_);
    INTELL_VOICE_LOG_INFO("enter, type:%{public}d", type);
    if (deathRecipientObj_.count(type) == 0 || deathRecipientObj_[type] == nullptr) {
        INTELL_VOICE_LOG_ERROR("death obj is nullptr, type:%{public}d", type);
        return false;
    }
    if (proxyDeathRecipient_.count(type) == 0 || proxyDeathRecipient_[type] == nullptr) {
        INTELL_VOICE_LOG_ERROR("death recipient is nullptr, type:%{public}d", type);
        deathRecipientObj_.erase(type);
        return false;
    }

    auto ret = deathRecipientObj_[type]->RemoveDeathRecipient(proxyDeathRecipient_[type]);
    deathRecipientObj_.erase(type);
    proxyDeathRecipient_.erase(type);
    return ret;
}

bool IntellVoiceServiceManager::IsEngineExist(IntellVoiceEngineType type)
{
    sptr<EngineBase> engine = GetEngine(type, engines_);
    if (engine != nullptr) {
        INTELL_VOICE_LOG_INFO("engine exist, type:%{public}d", type);
        return true;
    }

    if (type == INTELL_VOICE_UPDATE && GetUpdateState()) {
        INTELL_VOICE_LOG_ERROR("update is running");
        return true;
    }

    return false;
}

void IntellVoiceServiceManager::ProcBreathModel()
{
    INTELL_VOICE_LOG_INFO("enter");
    auto triggerMgr = TriggerManager::GetInstance();
    if (triggerMgr == nullptr) {
        INTELL_VOICE_LOG_WARN("trigger manager is nullptr");
        return;
    }

    if (triggerMgr->GetModel(PROXIMAL_WAKEUP_MODEL_UUID) != nullptr) {
        INTELL_VOICE_LOG_INFO("proximal model is exist, do nothing");
        return;
    }

    std::shared_ptr<uint8_t> buffer = nullptr;
    uint32_t size = 0;
    if (!IntellVoiceUtil::ReadFile(WHISPER_MODEL_PATH, buffer, size)) {
        return;
    }

    auto model = std::make_shared<GenericTriggerModel>(PROXIMAL_WAKEUP_MODEL_UUID, 1,
        TriggerModel::TriggerModelType::PROXIMAL_WAKEUP_TYPE);
    if (model == nullptr) {
        INTELL_VOICE_LOG_ERROR("model is nullptr");
        return;
    }

    model->SetData(buffer.get(), size);
    triggerMgr->UpdateModel(model);
}

bool IntellVoiceServiceManager::CreateUpdateEngine(const std::string &param)
{
    sptr<IIntellVoiceEngine> updateEngine = CreateEngine(INTELL_VOICE_UPDATE, param);
    if (updateEngine == nullptr) {
        INTELL_VOICE_LOG_ERROR("updateEngine is nullptr");
        return false;
    }

    return true;
}

void IntellVoiceServiceManager::ReleaseUpdateEngine()
{
    ReleaseEngine(INTELL_VOICE_UPDATE);
}

void IntellVoiceServiceManager::HandleUpdateComplete(UpdateState result, const std::string &param)
{
    TaskExecutor::AddAsyncTask([this, result, param]() {
        bool isLast = false;
        UpdateEngineController::UpdateCompleteProc(result, param, isLast);
        if ((IsEngineExist(INTELL_VOICE_ENROLL)) || (!isLast)) {
            INTELL_VOICE_LOG_INFO("enroll engine is existed, or is not last:%{public}d", isLast);
            return;
        }
        SwitchOnProc(VOICE_WAKEUP_MODEL_UUID, true);
        SwitchOnProc(PROXIMAL_WAKEUP_MODEL_UUID, true);
        UnloadIntellVoiceService();
    });
}

void IntellVoiceServiceManager::HandleUpdateRetry()
{
    TaskExecutor::AddAsyncTask([this]() {
        if (UpdateEngineController::UpdateRetryProc()) {
            INTELL_VOICE_LOG_INFO("retry to update");
            return;
        }
        if (IsEngineExist(INTELL_VOICE_ENROLL)) {
            INTELL_VOICE_LOG_INFO("enroll engine is existed, do nothing");
            return;
        }
        SwitchOnProc(VOICE_WAKEUP_MODEL_UUID, true);
        SwitchOnProc(PROXIMAL_WAKEUP_MODEL_UUID, true);
        UnloadIntellVoiceService();
    });
}

void IntellVoiceServiceManager::SetImproveParam(sptr<EngineBase> engine)
{
    if (engine == nullptr) {
        return;
    }

    // std::string status = QuerySwitchStatus(IMPROVE_KEY) ? "true" : "false";
    std::string status = "true";
    engine->SetParameter("userImproveOn=" + status);
}

std::string IntellVoiceServiceManager::GetParameter(const std::string &key)
{
    std::string val = "";

    if (key == "isEnrolled") {
        HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();
        val = historyInfoMgr.GetWakeupVesion().empty() ? "true" : "false";
        INTELL_VOICE_LOG_INFO("get is enroll result %{public}s", val.c_str());
    } else if (key == "isNeedReEnroll") {
        val = UpdateEngineUtils::IsVersionUpdate() ? "true" : "false";
        INTELL_VOICE_LOG_INFO("get nedd reenroll result %{public}s", val.c_str());
    }

    return val;
}

int32_t IntellVoiceServiceManager::GetWakeupSourceFilesList(std::vector<std::string>& cloneFiles)
{
    return EngineHostManager::GetInstance().GetWakeupSourceFilesList(cloneFiles);
}

int32_t IntellVoiceServiceManager::GetWakeupSourceFile(const std::string &filePath, std::vector<uint8_t> &buffer)
{
    return EngineHostManager::GetInstance().GetWakeupSourceFile(filePath, buffer);
}

int32_t IntellVoiceServiceManager::SendWakeupFile(const std::string &filePath, const std::vector<uint8_t> &buffer)
{
    if (buffer.data() == nullptr) {
        INTELL_VOICE_LOG_ERROR("send update callback is nullptr");
    }

    return EngineHostManager::GetInstance().SendWakeupFile(filePath, buffer);
}

sptr<IIntellVoiceEngine> IntellVoiceServiceManager::HandleCreateEngine(IntellVoiceEngineType type)
{
    return TaskExecutor::AddSyncTask([this, type]() -> sptr<IIntellVoiceEngine> {
        return CreateEngine(type);
    });
}

int32_t IntellVoiceServiceManager::HandleReleaseEngine(IntellVoiceEngineType type)
{
    return TaskExecutor::AddSyncTask([this, type]() -> int32_t { return ReleaseEngine(type); });
}

int32_t IntellVoiceServiceManager::CloneUpdate(const std::string &wakeupInfo, const sptr<IRemoteObject> &object)
{
    sptr<IIntelligentVoiceUpdateCallback> updateCallback = iface_cast<IIntelligentVoiceUpdateCallback>(object);
    if (updateCallback == nullptr) {
        INTELL_VOICE_LOG_ERROR("update callback is nullptr");
        return -1;
    }

    if (wakeupInfo.empty()) {
        INTELL_VOICE_LOG_ERROR("clone info empty");
        return -1;
    }

    std::shared_ptr<CloneUpdateStrategy> cloneStrategy =
        std::make_shared<CloneUpdateStrategy>(wakeupInfo, updateCallback);
    if (cloneStrategy == nullptr) {
        INTELL_VOICE_LOG_ERROR("clone strategy is nullptr");
        return -1;
    }

    INTELL_VOICE_LOG_INFO("enter");
    std::shared_ptr<IUpdateStrategy> strategy = std::dynamic_pointer_cast<IUpdateStrategy>(cloneStrategy);
    return CreateUpdateEngineUntilTime(strategy);
}

int32_t IntellVoiceServiceManager::HandleCloneUpdate(const std::string &wakeupInfo, const sptr<IRemoteObject> &object)
{
    return TaskExecutor::AddSyncTask([this, wakeupInfo, object = std::move(object)]() -> int32_t {
        return CloneUpdate(wakeupInfo, object);
    });
}

int32_t IntellVoiceServiceManager::SilenceUpdate()
{
    std::shared_ptr<SilenceUpdateStrategy> silenceStrategy = std::make_shared<SilenceUpdateStrategy>("");
    if (silenceStrategy == nullptr) {
        INTELL_VOICE_LOG_ERROR("silence strategy is nullptr");
        return -1;
    }

    INTELL_VOICE_LOG_INFO("enter");
    std::shared_ptr<IUpdateStrategy> strategy = std::dynamic_pointer_cast<IUpdateStrategy>(silenceStrategy);
    return CreateUpdateEngineUntilTime(strategy);
}

void IntellVoiceServiceManager::HandleSilenceUpdate()
{
    TaskExecutor::AddAsyncTask([this]() { SilenceUpdate(); });
}

int32_t IntellVoiceServiceManager::SwitchOnProc(int32_t uuid, bool needUpdateAdapter)
{
    INTELL_VOICE_LOG_INFO("enter, uuid:%{public}d", uuid);
    if ((IsEngineExist(INTELL_VOICE_ENROLL)) || (IsEngineExist(INTELL_VOICE_UPDATE))) {
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

void IntellVoiceServiceManager::HandleSwitchOn(bool isAsync, int32_t uuid, bool needUpdateAdapter)
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

int32_t IntellVoiceServiceManager::SwitchOffProc(int32_t uuid)
{
    INTELL_VOICE_LOG_INFO("enter, uuid:%{public}d", uuid);
    if ((IsEngineExist(INTELL_VOICE_ENROLL)) || (IsEngineExist(INTELL_VOICE_UPDATE))) {
        INTELL_VOICE_LOG_INFO("enroll engine or update engine exist, do nothing");
        return 0;
    }

    if (QuerySwitchByUuid(uuid)) {
        INTELL_VOICE_LOG_INFO("switch is on, do nothing, uuid is %{public}d", uuid);
        return 0;
    }

    ReleaseServiceObject(uuid);
    INTELL_VOICE_LOG_INFO("exit, uuid:%{public}d", uuid);
    return 0;
}

void IntellVoiceServiceManager::HandleSwitchOff(bool isAsync, int32_t uuid)
{
    INTELL_VOICE_LOG_INFO("enter, isAsync:%{public}d, uuid:%{public}d", isAsync, uuid);
    if (!isAsync) {
        TaskExecutor::AddSyncTask([this, uuid]() -> int32_t { return SwitchOffProc(uuid); });
    } else {
        TaskExecutor::AddAsyncTask([this, uuid]() { SwitchOffProc(uuid); });
    }
}

void IntellVoiceServiceManager::HandleCloseWakeupSource()
{
    INTELL_VOICE_LOG_INFO("enter");
    TaskExecutor::AddAsyncTask([this]() { StartDetection(VOICE_WAKEUP_MODEL_UUID); });
    TaskExecutor::AddAsyncTask([this]() { StartDetection(PROXIMAL_WAKEUP_MODEL_UUID); });
}

bool IntellVoiceServiceManager::IsNeedToUnloadService()
{
    if ((IsEngineExist(INTELL_VOICE_ENROLL)) || (IsEngineExist(INTELL_VOICE_UPDATE))) {
        INTELL_VOICE_LOG_INFO("enroll engine or update engine exist, no need to unload service");
        return false;
    }

    if ((QuerySwitchStatus(WAKEUP_KEY)) || (QuerySwitchStatus(WHISPER_KEY))) {
        INTELL_VOICE_LOG_INFO("switch is on, no need to unload service");
        return false;
    }

    return true;
}

int32_t IntellVoiceServiceManager::UnloadIntellVoiceService()
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

void IntellVoiceServiceManager::HandleUnloadIntellVoiceService(bool isAsync)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (isAsync) {
        TaskExecutor::AddAsyncTask([this]() { UnloadIntellVoiceService(); });
    } else {
        TaskExecutor::AddSyncTask([this]() -> int32_t { return UnloadIntellVoiceService(); });
    }
}

bool IntellVoiceServiceManager::HandleOnIdle()
{
    return TaskExecutor::AddSyncTask([this]() -> bool { return IsNeedToUnloadService(); });
}

void IntellVoiceServiceManager::HandleServiceStop()
{
    TaskExecutor::AddSyncTask([this]() -> int32_t { return ServiceStopProc(); });
}

void IntellVoiceServiceManager::HandleHeadsetHostDie()
{
    TaskExecutor::AddAsyncTask([this]() {
        auto engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
        if (engine != nullptr) {
            engine->NotifyHeadsetHostEvent(HEADSET_HOST_ON);
        }
    });
}
}  // namespace IntellVoiceEngine
}  // namespace OHOS
