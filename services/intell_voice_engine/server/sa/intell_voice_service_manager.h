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
#include "intell_voice_engine_arbitration.h"
#include "intell_voice_death_recipient.h"
#include "update_engine_controller.h"

namespace OHOS {
namespace IntellVoiceEngine {
class IntellVoiceServiceManager : private IntellVoiceEngineArbitration, private UpdateEngineController {
public:
    ~IntellVoiceServiceManager();
    static std::unique_ptr<IntellVoiceServiceManager> &GetInstance();
    static int32_t GetEnrollModelUuid()
    {
        return g_enrollModelUuid;
    }
    static void SetEnrollResult(IntellVoiceEngineType type, bool result)
    {
        if (type >= ENGINE_TYPE_BUT) {
            return;
        }

        enrollResult_[type].store(result);
    }
    static bool GetEnrollResult(IntellVoiceEngineType type)
    {
        if (type >= ENGINE_TYPE_BUT) {
            return false;
        }

        return enrollResult_[type].load();
    }
    sptr<IIntellVoiceEngine> CreateEngine(IntellVoiceEngineType type);
    int32_t ReleaseEngine(IntellVoiceEngineType type);

    void OnServiceStart();
    void OnServiceStop();
    void CreateSwitchProvider();
    void ReleaseSwitchProvider();
    void StartDetection();
    void StopDetection();
    bool QuerySwitchStatus();
    void UnloadIntellVoiceService();

    bool RegisterProxyDeathRecipient(IntellVoiceEngineType type, const sptr<IRemoteObject> &object);
    bool DeregisterProxyDeathRecipient(IntellVoiceEngineType type);

    using UpdateEngineController::SaveWakeupVesion;
    using UpdateEngineController::OnUpdateComplete;
    using UpdateEngineController::CreateUpdateEngineUntilTime;

private:
    IntellVoiceServiceManager();
    void OnSwitchChange();
    void OnDetected();
    void CreateDetector();

    sptr<IIntellVoiceEngine> CreateEngineInner(IntellVoiceEngineType type);
    int32_t ReleaseEngineInner(IntellVoiceEngineType type);
    bool CreateOrResetWakeupEngine();
    bool IsEngineExist(IntellVoiceEngineType type);
    void ReleaseUpdateEngine() override;
    bool CreateUpdateEngine() override;
    void UpdateCompleteHandler(UpdateState result, bool isLast) override;
private:
    static const int32_t g_enrollModelUuid;
    static std::unique_ptr<IntellVoiceServiceManager> g_intellVoiceServiceMgr;
    static std::atomic<bool> enrollResult_[ENGINE_TYPE_BUT];
    std::atomic<bool> isServiceUnloaded_ = false;
    std::mutex engineMutex_;
    std::mutex detectorMutex_;
    std::mutex switchMutex_;
    std::map<IntellVoiceEngineType, sptr<EngineBase>> engines_;
    std::shared_ptr<IntellVoiceTrigger::TriggerDetector> detector_ = nullptr;
    sptr<SwitchObserver> switchObserver_ = nullptr;
    IntellVoiceUtils::UniqueProductType<SwitchProvider> switchProvider_ =
        IntellVoiceUtils::UniqueProductType<SwitchProvider> {nullptr, nullptr};
    std::map<IntellVoiceEngineType, sptr<IntellVoiceUtils::IntellVoiceDeathRecipient>> proxyDeathRecipient_;
    std::map<IntellVoiceEngineType, sptr<IRemoteObject>> deathRecipientObj_;
};
}  // namespace IntellVoice
}  // namespace OHOS
#endif
