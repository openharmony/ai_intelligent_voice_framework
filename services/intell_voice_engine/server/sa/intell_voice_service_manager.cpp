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
const std::string BREATH_MODEL_PATH = "/sys_prod/variant/region_comm/china/etc/intellvoice/wakeup/wakeup_dsp_config";

std::atomic<bool> IntellVoiceServiceManager::enrollResult_[ENGINE_TYPE_BUT] = {false, false, false};

std::unique_ptr<IntellVoiceServiceManager> IntellVoiceServiceManager::g_intellVoiceServiceMgr =
    std::unique_ptr<IntellVoiceServiceManager>(new (std::nothrow) IntellVoiceServiceManager());

IntellVoiceServiceManager::IntellVoiceServiceManager()
{}

IntellVoiceServiceManager::~IntellVoiceServiceManager()
{
    engines_.clear();
}

std::unique_ptr<IntellVoiceServiceManager> &IntellVoiceServiceManager::GetInstance()
{
    return g_intellVoiceServiceMgr;
}

sptr<IIntellVoiceEngine> IntellVoiceServiceManager::CreateEngine(IntellVoiceEngineType type, std::string param)
{
    INTELL_VOICE_LOG_INFO("enter, type:%{public}d", type);
    if (type == INTELL_VOICE_ENROLL) {
        StopDetection();
    }

    std::lock_guard<std::mutex> lock(engineMutex_);

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

sptr<IIntellVoiceEngine> IntellVoiceServiceManager::CreateEngineInner(IntellVoiceEngineType type, std::string param)
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
    std::lock_guard<std::mutex> lock(engineMutex_);
    auto ret = ReleaseEngineInner(type);
    if (ret != 0) {
        return ret;
    }

    if (type == INTELL_VOICE_ENROLL) {
        if ((!GetEnrollResult(type)) && (!QuerySwitchStatus(WAKEUP_KEY))) {
            INTELL_VOICE_LOG_INFO("enroll fail, unloadservice");
            std::thread(&IntellVoiceServiceManager::UnloadIntellVoiceService, this).detach();
            return 0;
        }

        auto triggerMgr = TriggerManager::GetInstance();
        if (triggerMgr == nullptr) {
            INTELL_VOICE_LOG_WARN("trigger manager is nullptr");
            return 0;
        }
        auto model = triggerMgr->GetModel(VOICE_WAKEUP_MODEL_UUID);
        if (model == nullptr) {
            INTELL_VOICE_LOG_WARN("no model");
            return 0;
        }

        if (!CreateOrResetWakeupEngine()) {
            return 0;
        }

        CreateDetector(VOICE_WAKEUP_MODEL_UUID);
        StartDetection(VOICE_WAKEUP_MODEL_UUID);
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
    if (isServiceUnloaded_.load()) {
        INTELL_VOICE_LOG_INFO("service is unloading");
        return;
    }

    switchProvider_ = UniquePtrFactory<SwitchProvider>::CreateInstance();
    if (switchProvider_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("switchProvider_ is nullptr");
        return;
    }
    RegisterObserver(WAKEUP_KEY);
    RegisterObserver(BREATH_KEY);
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
    if (isServiceUnloaded_.load()) {
        INTELL_VOICE_LOG_INFO("service is unloading");
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
    std::thread([uuid, this]() {
        if (IsEngineExist(INTELL_VOICE_ENROLL) || IsEngineExist(INTELL_VOICE_UPDATE)) {
            INTELL_VOICE_LOG_INFO("enroll engine or update engine exist, no need to start to recognize");
            return;
        }

        for (uint32_t cnt = 1; cnt <= MAX_ATTEMPT_CNT; ++cnt) {
            {
                std::lock_guard<std::mutex> lock(detectorMutex_);
                if ((detector_.count(uuid) == 0) || (detector_[uuid] == nullptr)) {
                    INTELL_VOICE_LOG_INFO("detector is not existed, uuid:%{public}d", uuid);
                    return;
                }

                if (isServiceUnloaded_.load()) {
                    INTELL_VOICE_LOG_INFO("service is unloading");
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
    }).detach();
}

void IntellVoiceServiceManager::StopDetection()
{
    std::lock_guard<std::mutex> lock(detectorMutex_);
    for (auto it : detector_) {
        if (it.second != nullptr) {
            it.second->StopRecognition();
        }
    }
}

void IntellVoiceServiceManager::OnDetected(int32_t uuid)
{
    std::lock_guard<std::mutex> lock(engineMutex_);
    sptr<EngineBase> engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
    if (engine == nullptr) {
        INTELL_VOICE_LOG_ERROR("wakeup engine is not existed");
        return;
    }

    engine->OnDetected(uuid);
}

void IntellVoiceServiceManager::CreateAndStartServiceObject(int32_t uuid)
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
    {
        std::lock_guard<std::mutex> lock(engineMutex_);
        if (isServiceUnloaded_.load()) {
            INTELL_VOICE_LOG_INFO("service is unloading");
            return;
        }
        if (CreateEngineInner(INTELL_VOICE_WAKEUP) == nullptr) {
            INTELL_VOICE_LOG_ERROR("failed to create wakeup engine");
            return;
        }
    }
    CreateDetector(uuid);
    StartDetection(uuid);
}

void IntellVoiceServiceManager::OnServiceStart()
{
    CreateAndStartServiceObject(VOICE_WAKEUP_MODEL_UUID);
}

void IntellVoiceServiceManager::OnServiceStop()
{
    {
        std::lock_guard<std::mutex> lock(detectorMutex_);
        for (auto it : detector_) {
            if (it.second != nullptr) {
                it.second->UnloadTriggerModel();
            }
        }
    }
    {
        std::lock_guard<std::mutex> lock(engineMutex_);
        sptr<EngineBase> wakeupEngine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
        if (wakeupEngine == nullptr) {
            INTELL_VOICE_LOG_INFO("wakeup engine is not existed");
            return;
        }
        wakeupEngine->Detach();
    }
}

bool IntellVoiceServiceManager::QuerySwitchStatus(const std::string &key)
{
    std::lock_guard<std::mutex> lock(switchMutex_);
    if (key != WAKEUP_KEY && key != IMPROVE_KEY) {
        INTELL_VOICE_LOG_ERROR("invalid key :%{public}s", key.c_str());
        return false;
    }

    if (switchProvider_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("switchProvider_ is nullptr");
        return false;
    }
    return switchProvider_->QuerySwitchStatus(key);
}

void IntellVoiceServiceManager::UnloadIntellVoiceService()
{
    isServiceUnloaded_.store(true);

    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("Failed to get systemabilitymanager.");
        return;
    }
    int32_t ret = systemAbilityMgr->UnloadSystemAbility(INTELL_VOICE_SERVICE_ID);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("Failed to unload intellvoice service, ret: %{public}d", ret);
        return;
    }
    INTELL_VOICE_LOG_INFO("Success to unload intellvoice service");
}

void IntellVoiceServiceManager::OnWakeupSwitchChange()
{
    if (!QuerySwitchStatus(WAKEUP_KEY)) {
        INTELL_VOICE_LOG_INFO("switch off");
        if (!IsEngineExist(INTELL_VOICE_ENROLL) && !IsEngineExist(INTELL_VOICE_UPDATE)) {
            UnloadIntellVoiceService();
        }
    } else {
        INTELL_VOICE_LOG_INFO("switch on");
        if (!IsEngineExist(INTELL_VOICE_ENROLL) && !IsEngineExist(INTELL_VOICE_UPDATE)) {
            OnServiceStart();
        }
    }
}

void IntellVoiceServiceManager::OnSwitchChange(const std::string switchKey)
{
    if (switchKey == WAKEUP_KEY) {
        OnWakeupSwitchChange();
    } else if (switchKey == IMPROVE_KEY) {
        std::lock_guard<std::mutex> lock(engineMutex_);
        auto engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
        if (engine != nullptr) {
            SetImproveParam(engine);
        }
    } else if (switchKey == SHORTWORD_KEY) {
        INTELL_VOICE_LOG_INFO("shot word switch change");
    }
}

bool IntellVoiceServiceManager::RegisterProxyDeathRecipient(IntellVoiceEngineType type,
    const sptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(engineMutex_);
    INTELL_VOICE_LOG_INFO("enter, type:%{public}d", type);
    deathRecipientObj_[type] = object;
    if (type == INTELL_VOICE_ENROLL) {
        proxyDeathRecipient_[type] = new (std::nothrow) IntellVoiceDeathRecipient([&]() {
            INTELL_VOICE_LOG_INFO("receive enroll proxy death recipient, release enroll engine");
            ReleaseEngine(INTELL_VOICE_ENROLL);
        });
        if (proxyDeathRecipient_[type] == nullptr) {
            INTELL_VOICE_LOG_ERROR("create death recipient failed");
            return false;
        }
    } else if (type == INTELL_VOICE_WAKEUP) {
        proxyDeathRecipient_[type] = new (std::nothrow) IntellVoiceDeathRecipient([&]() {
            INTELL_VOICE_LOG_INFO("receive wakeup proxy death recipient, clear wakeup engine callback");
            std::lock_guard<std::mutex> lock(engineMutex_);
            sptr<EngineBase> engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
            if (engine != nullptr) {
                INTELL_VOICE_LOG_INFO("clear wakeup engine callback");
                engine->SetCallback(nullptr);
            }
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
    std::lock_guard<std::mutex> lock(engineMutex_);
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
    std::lock_guard<std::mutex> lock(engineMutex_);
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
    std::shared_ptr<uint8_t> buffer = nullptr;
    uint32_t size = 0;
    if (!IntellVoiceUtil::ReadFile(BREATH_MODEL_PATH, buffer, size)) {
        return;
    }

    auto model = std::make_shared<GenericTriggerModel>(PROXIMAL_WAKEUP_MODEL_UUID, 1,
        TriggerModel::TriggerModelType::PROXIMAL_WAKEUP_TYPE);
    if (model == nullptr) {
        INTELL_VOICE_LOG_ERROR("model is nullptr");
        return;
    }

    model->SetData(buffer.get(), size);
    auto triggerMgr = TriggerManager::GetInstance();
    if (triggerMgr != nullptr) {
        triggerMgr->UpdateModel(model);
    }
}

bool IntellVoiceServiceManager::CreateUpdateEngine(std::string param)
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

void IntellVoiceServiceManager::UpdateCompleteHandler(UpdateState result, bool isLast)
{
    sptr<EngineBase> engineEnroll = GetEngine(INTELL_VOICE_ENROLL, engines_);
    if (engineEnroll != nullptr) {
        INTELL_VOICE_LOG_INFO("enroll engine is existed");
        return;
    }

    if (result == UpdateState::UPDATE_STATE_COMPLETE_FAIL && isLast) {
        INTELL_VOICE_LOG_INFO("update failed");
    }

    if (QuerySwitchStatus(WAKEUP_KEY)) {
        INTELL_VOICE_LOG_INFO("restart wakeup");
        auto triggerMgr = TriggerManager::GetInstance();
        if (triggerMgr == nullptr) {
            INTELL_VOICE_LOG_WARN("trigger manager is nullptr");
            return;
        }
        auto model = triggerMgr->GetModel(VOICE_WAKEUP_MODEL_UUID);
        if (model == nullptr) {
            INTELL_VOICE_LOG_WARN("no model");
            return;
        }

        if (!CreateOrResetWakeupEngine()) {
            return;
        }

        CreateDetector(VOICE_WAKEUP_MODEL_UUID);
        StartDetection(VOICE_WAKEUP_MODEL_UUID);
    } else {
        if (isLast) {
            INTELL_VOICE_LOG_INFO("unload service");
            UnloadIntellVoiceService();
        }
    }
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
}  // namespace IntellVoiceEngine
}  // namespace OHOS
