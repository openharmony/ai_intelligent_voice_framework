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
#include "intell_voice_generic_factory.h"
#include "trigger_detector_callback.h"
#include "memory_guard.h"

using namespace OHOS::IntellVoiceTrigger;
using namespace OHOS::IntellVoiceUtils;
#define LOG_TAG "IntellVoiceServiceManager"

namespace OHOS {
namespace IntellVoiceEngine {
const int32_t IntellVoiceServiceManager::g_enrollModelUuid = 1;

std::unique_ptr<IntellVoiceServiceManager> IntellVoiceServiceManager::g_intellVoiceServiceMgr =
    std::unique_ptr<IntellVoiceServiceManager>(new (std::nothrow) IntellVoiceServiceManager());

IntellVoiceServiceManager::IntellVoiceServiceManager()
{
    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    historyInfoMgr_ = std::make_unique<HistoryInfoMgr>();
    if (historyInfoMgr_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("historyInfoMgr_ is nullptr");
    }
}

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

    if (ApplyArbitration(type, ENGINE_EVENT_CREATE) != ARBITRATION_OK) {
        INTELL_VOICE_LOG_ERROR("policy manager reject create engine, type:%{public}d", type);
        return nullptr;
    }

    return CreateEngineInner(type);
}

sptr<IIntellVoiceEngine> IntellVoiceServiceManager::CreateEngineInner(IntellVoiceEngineType type)
{
    INTELL_VOICE_LOG_INFO("create engine enter, type: %{public}d", type);
    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    auto it = engines_.find(type);
    if (it != engines_.end() && it->second != nullptr) {
        return it->second;
    }

    sptr<EngineBase> engine = EngineFactory::CreateEngineInst(type);
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

        auto wakeupEngine = CreateEngineInner(INTELL_VOICE_WAKEUP);
        if (wakeupEngine == nullptr) {
            INTELL_VOICE_LOG_WARN("failed to create wakeup engine");
            return 0;
        }
        CreateDetector();
        if (switchProvider_->QuerySwitchStatus()) {
            StartDetection();
        }
    }

    return 0;
}

int32_t IntellVoiceServiceManager::ReleaseEngineInner(IntellVoiceEngineType type)
{
    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    auto it = engines_.find(type);
    if (it == engines_.end()) {
        INTELL_VOICE_LOG_ERROR("there is no engine(%{public}d) in list", type);
        return -1;
    }

    it->second = nullptr;
    engines_.erase(type);
    return 0;
}

void IntellVoiceServiceManager::CreateSwitchProvider()
{
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

    if (switchObserver_ != nullptr) {
        switchProvider_->RegisterObserver(switchObserver_);
    }
}

void IntellVoiceServiceManager::ReleaseSwitchProvider()
{
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
    std::lock_guard<std::mutex> lock(detectorMutex_);
    if (detector_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("detector_ is nullptr");
        return;
    }

    detector_->StartRecognition();
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
        auto it = engines_.find(INTELL_VOICE_WAKEUP);
        if ((it == engines_.end()) || (it->second == nullptr)) {
            INTELL_VOICE_LOG_ERROR("wakeup engine is not existed");
            return;
        }

        engine = it->second;
    }

    engine->OnDetected();
}

void IntellVoiceServiceManager::OnUserUnlock()
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
        sptr<EngineBase> wakeupEngine = EngineFactory::CreateEngineInst(INTELL_VOICE_WAKEUP);
        if (wakeupEngine == nullptr) {
            INTELL_VOICE_LOG_ERROR("wakeupEngine is nullptr");
            return;
        }
        engines_[INTELL_VOICE_WAKEUP] = wakeupEngine;
    }

    CreateDetector();
    if (switchProvider_->QuerySwitchStatus()) {
        StartDetection();
    }
}

void IntellVoiceServiceManager::OnSwitchChange()
{
    if (switchProvider_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("switchProvider_ is nullptr");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(engineMutex_);
        auto it = engines_.find(INTELL_VOICE_ENROLL);
        if ((it != engines_.end()) && (it->second != nullptr)) {
            INTELL_VOICE_LOG_INFO("enroll engine is existed");
            return;
        }
    }

    if (switchProvider_->QuerySwitchStatus()) {
        INTELL_VOICE_LOG_INFO("switch on");
        StartDetection();
    } else {
        INTELL_VOICE_LOG_INFO("switch off");
        StopDetection();
    }
}

int32_t IntellVoiceServiceManager::ApplyArbitration(IntellVoiceEngineType type, EngineEvent event)
{
    INTELL_VOICE_LOG_INFO("enter");
    switch (event) {
        case ENGINE_EVENT_CREATE:
            return CreateArbitration(type);
        case ENGINE_EVENT_START:
            return StartArbitration(type);
        default:
            INTELL_VOICE_LOG_INFO("unknown engine event:%{public}d", event);
            break;
    }
    return ARBITRATION_OK;
}

int32_t IntellVoiceServiceManager::CreateArbitration(IntellVoiceEngineType type)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (type == INTELL_VOICE_ENROLL) {
        auto wakeupEngineIt = engines_.find(INTELL_VOICE_WAKEUP);
        if (wakeupEngineIt != engines_.end()) {
            HandlePreemption(wakeupEngineIt->second);
        }

        auto enrollEngineIt = engines_.find(INTELL_VOICE_ENROLL);
        if (enrollEngineIt != engines_.end()) {
            HandleReplace(enrollEngineIt->second);
        }
    }
    return ARBITRATION_OK;
}

int32_t IntellVoiceServiceManager::StartArbitration(IntellVoiceEngineType type)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (type == INTELL_VOICE_WAKEUP) {
        auto it = engines_.find(INTELL_VOICE_ENROLL);
        if (it != engines_.end()) {
            return ARBITRATION_REJECT;
        }
    }
    return ARBITRATION_OK;
}

void IntellVoiceServiceManager::HandlePreemption(sptr<IIntellVoiceEngine> currentEngine)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (currentEngine == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to stop current engine, current engine is null");
        return;
    }
    currentEngine->Stop();
}

void IntellVoiceServiceManager::HandleReplace(sptr<IIntellVoiceEngine> currentEngine)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (currentEngine == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to detach current engine, current engine is null");
        return;
    }
    currentEngine->Detach();
    currentEngine = nullptr;
}
}  // namespace IntellVoiceEngine
}  // namespace OHOS
