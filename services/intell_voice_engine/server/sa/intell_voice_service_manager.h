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
#ifndef SERVICE_MANAGER_H
#define SERVICE_MANAGER_H
#include <mutex>
#include <map>
#include <atomic>
#include "engine_base.h"
#include "trigger_detector.h"
#include "switch_observer.h"
#include "switch_provider.h"
#include "history_info_mgr.h"

namespace OHOS {
namespace IntellVoiceEngine {
enum EngineEvent {
    ENGINE_EVENT_CREATE = 0,
    ENGINE_EVENT_START,
};

enum ArbitrationResult {
    ARBITRATION_OK = 0,
    ARBITRATION_REJECT,
};

class IntellVoiceServiceManager {
public:
    ~IntellVoiceServiceManager();
    static std::unique_ptr<IntellVoiceServiceManager> &GetInstance();
    static int32_t GetEnrollModelUuid()
    {
        return g_enrollModelUuid;
    }
    static void SetEnrollResult(bool result)
    {
        enrollResult_.store(result);
    }
    static bool GetEnrollResult()
    {
        return enrollResult_.load();
    }
    sptr<IIntellVoiceEngine> CreateEngine(IntellVoiceEngineType type);
    int32_t ReleaseEngine(IntellVoiceEngineType type);
    const std::unique_ptr<HistoryInfoMgr> &GetHistoryInfoMgr() const
    {
        return historyInfoMgr_;
    }

    void OnServiceStart();
    void CreateSwitchProvider();
    void ReleaseSwitchProvider();
    void StartDetection();
    void StopDetection();
    bool QuerySwitchStatus();
    void UnloadIntellVoiceService();

    int32_t ApplyArbitration(IntellVoiceEngineType type, EngineEvent event);

private:
    IntellVoiceServiceManager();
    void OnSwitchChange();
    void OnDetected();
    void CreateDetector();

    int32_t CreateArbitration(IntellVoiceEngineType type);
    int32_t StartArbitration(IntellVoiceEngineType type);
    void HandlePreemption(sptr<IIntellVoiceEngine> currentEngine);
    void HandleReplace(sptr<IIntellVoiceEngine> currentEngine);

    sptr<IIntellVoiceEngine> CreateEngineInner(IntellVoiceEngineType type);
    int32_t ReleaseEngineInner(IntellVoiceEngineType type);

private:
    static const int32_t g_enrollModelUuid;
    static std::unique_ptr<IntellVoiceServiceManager> g_intellVoiceServiceMgr;
    static std::atomic<bool> enrollResult_;
    std::mutex engineMutex_;
    std::mutex detectorMutex_;
    std::map<IntellVoiceEngineType, sptr<EngineBase>> engines_;
    std::shared_ptr<IntellVoiceTrigger::TriggerDetector> detector_ = nullptr;
    sptr<SwitchObserver> switchObserver_ = nullptr;
    IntellVoiceUtils::UniqueProductType<SwitchProvider> switchProvider_ =
        IntellVoiceUtils::UniqueProductType<SwitchProvider> {nullptr, nullptr};
    std::unique_ptr<HistoryInfoMgr> historyInfoMgr_ = nullptr;
};
}  // namespace IntellVoice
}  // namespace OHOS
#endif