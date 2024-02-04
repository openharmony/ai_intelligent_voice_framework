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

#include "intell_voice_log.h"
#include "engine_factory.h"
#include "wakeup_engine.h"
#include "trigger_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "intell_voice_generic_factory.h"
#include "trigger_detector_callback.h"
#include "memory_guard.h"
#include "iproxy_broker.h"
#include "string_util.h"

#define LOG_TAG "IntellVoiceServiceManager"

using namespace OHOS::IntellVoiceTrigger;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;

namespace OHOS {
namespace IntellVoiceEngine {
static constexpr int32_t MAX_ATTEMPT_CNT = 10;
const int32_t IntellVoiceServiceManager::g_enrollModelUuid = 1;

std::atomic<bool> IntellVoiceServiceManager::enrollResult_[ENGINE_TYPE_BUT] = {false, false, false};

std::unique_ptr<IntellVoiceServiceManager> IntellVoiceServiceManager::g_intellVoiceServiceMgr =
    std::unique_ptr<IntellVoiceServiceManager>(new (std::nothrow) IntellVoiceServiceManager());

IntellVoiceServiceManager::IntellVoiceServiceManager()
{}

IntellVoiceServiceManager::~IntellVoiceServiceManager()
{
    engines_.clear();
    switchObserver_ = nullptr;
    switchProvider_ = nullptr;
}

std::unique_ptr<IntellVoiceServiceManager> &IntellVoiceServiceManager::GetInstance()
{
    return g_intellVoiceServiceMgr;
}

sptr<IIntellVoiceEngine> IntellVoiceServiceManager::CreateEngine(IntellVoiceEngineType type)
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

    return CreateEngineInner(type);
}

sptr<IIntellVoiceEngine> IntellVoiceServiceManager::CreateEngineInner(IntellVoiceEngineType type)
{
    INTELL_VOICE_LOG_INFO("create engine enter, type: %{public}d", type);
    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    sptr<EngineBase> engine = GetEngine(type, engines_);
    if (engine != nullptr) {
        return engine;
    }

    engine = EngineFactory::CreateEngineInst(type);
    if (engine == nullptr) {
        INTELL_VOICE_LOG_ERROR("create engine failed, type:%{public}d", type);
        return nullptr;
    }

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
        if ((!GetEnrollResult(type)) && (!QuerySwitchStatus())) {
            INTELL_VOICE_LOG_INFO("enroll fail, unloadservice");
            std::thread(&IntellVoiceServiceManager::UnloadIntellVoiceService, this).detach();
            return 0;
        }

        auto triggerMgr = TriggerManager::GetInstance();
        if (triggerMgr == nullptr) {
            INTELL_VOICE_LOG_WARN("trigger manager is nullptr");
            return 0;
        }
        auto model = triggerMgr->GetModel(IntellVoiceServiceManager::GetEnrollModelUuid());
        if (model == nullptr) {
            INTELL_VOICE_LOG_WARN("no model");
            return 0;
        }

        if (!CreateOrResetWakeupEngine()) {
            return 0;
        }

        CreateDetector();
        StartDetection();
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

    switchObserver_ = sptr<SwitchObserver>(new (std::nothrow) SwitchObserver());
    if (switchObserver_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("switchObserver_ is nullptr");
        return;
    }

    switchObserver_->SetUpdateFunc([]() {
        const auto &manager = IntellVoiceServiceManager::GetInstance();
        if (manager != nullptr) {
            manager->OnSwitchChange();
        }
    });

    switchProvider_ = UniquePtrFactory<SwitchProvider>::CreateInstance();
    if (switchProvider_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("switchProvider_ is nullptr");
        return;
    }

    switchProvider_->RegisterObserver(switchObserver_);
}

void IntellVoiceServiceManager::ReleaseSwitchProvider()
{
    std::lock_guard<std::mutex> lock(switchMutex_);
    if (switchProvider_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("switchProvider_ is nullptr");
        return;
    }

    if (switchObserver_ != nullptr) {
        switchProvider_->UnregisterObserver(switchObserver_);
    }

    switchProvider_ = nullptr;
}

void IntellVoiceServiceManager::CreateDetector()
{
    std::lock_guard<std::mutex> lock(detectorMutex_);
    if (isServiceUnloaded_.load()) {
        INTELL_VOICE_LOG_INFO("service is unloading");
        return;
    }

    if (detector_ != nullptr) {
        INTELL_VOICE_LOG_INFO("detector is already existed, no need to create");
        return;
    }

    std::shared_ptr<TriggerDetectorCallback> cb = std::make_shared<TriggerDetectorCallback>([&]() { OnDetected(); });
    if (cb == nullptr) {
        INTELL_VOICE_LOG_ERROR("cb is nullptr");
        return;
    }

    auto triggerMgr = TriggerManager::GetInstance();
    if (triggerMgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger manager is nullptr");
        return;
    }

    detector_ = triggerMgr->CreateTriggerDetector(1, cb);
    if (detector_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("detector_ is nullptr");
        return;
    }
}

void IntellVoiceServiceManager::StartDetection()
{
    std::thread([this]() {
        for (uint32_t cnt = 1; cnt <= MAX_ATTEMPT_CNT; ++cnt) {
            {
                std::lock_guard<std::mutex> lock(detectorMutex_);
                if (detector_ == nullptr) {
                    INTELL_VOICE_LOG_ERROR("detector_ is nullptr");
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
                    detector_->StartRecognition();
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
    if (detector_ == nullptr) {
        return;
    }

    detector_->StopRecognition();
}

void IntellVoiceServiceManager::OnDetected()
{
    sptr<EngineBase> engine = nullptr;

    {
        std::lock_guard<std::mutex> lock(engineMutex_);
        engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
        if (engine == nullptr) {
            INTELL_VOICE_LOG_ERROR("wakeup engine is not existed");
            return;
        }
    }

    engine->OnDetected();
}

void IntellVoiceServiceManager::OnServiceStart()
{
    auto triggerMgr = TriggerManager::GetInstance();
    if (triggerMgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger manager is nullptr");
        return;
    }
    auto model = triggerMgr->GetModel(IntellVoiceServiceManager::GetEnrollModelUuid());
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

    CreateDetector();
    StartDetection();
}

void IntellVoiceServiceManager::OnServiceStop()
{
    {
        std::lock_guard<std::mutex> lock(detectorMutex_);
        if (detector_ != nullptr) {
            detector_->UnloadTriggerModel();
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

bool IntellVoiceServiceManager::QuerySwitchStatus()
{
    std::lock_guard<std::mutex> lock(switchMutex_);
    if (switchProvider_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("switchProvider_ is nullptr");
        return false;
    }
    return switchProvider_->QuerySwitchStatus();
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

void IntellVoiceServiceManager::OnSwitchChange()
{
    if (!QuerySwitchStatus()) {
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

bool IntellVoiceServiceManager::CreateUpdateEngine()
{
    sptr<IIntellVoiceEngine> updateEngine = CreateEngine(INTELL_VOICE_UPDATE);
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

    if (QuerySwitchStatus()) {
        INTELL_VOICE_LOG_INFO("restart wakeup");
        auto triggerMgr = TriggerManager::GetInstance();
        if (triggerMgr == nullptr) {
            INTELL_VOICE_LOG_WARN("trigger manager is nullptr");
            return;
        }
        auto model = triggerMgr->GetModel(IntellVoiceServiceManager::GetEnrollModelUuid());
        if (model == nullptr) {
            INTELL_VOICE_LOG_WARN("no model");
            return;
        }

        if (!CreateOrResetWakeupEngine()) {
            return;
        }

        CreateDetector();
        StartDetection();
    } else {
        if (isLast) {
            INTELL_VOICE_LOG_INFO("unload service");
            UnloadIntellVoiceService();
        }
    }
}
}  // namespace IntellVoiceEngine
}  // namespace OHOS
