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
#include "data_operation_callback.h"
#include "i_intell_voice_update_callback.h"
#include "task_executor.h"
#include "engine_util.h"

namespace OHOS {
namespace IntellVoiceEngine {
constexpr int32_t VOICE_WAKEUP_MODEL_UUID = 1;
constexpr int32_t PROXIMAL_WAKEUP_MODEL_UUID = 2;
const std::string WAKEUP_KEY = "intell_voice_trigger_enabled";
const std::string WHISPER_KEY = "intell_voice_trigger_whisper";
const std::string IMPROVE_KEY = "intell_voice_improve_enabled";
const std::string SHORTWORD_KEY = "intell_voice_trigger_shortword";
const std::string SENSIBILITY_TEXT = "sensibility=";
const std::string KEY_GET_WAKEUP_FEATURE = "wakeup_features";


class IntellVoiceServiceManager : private IntellVoiceEngineArbitration,
    private UpdateEngineController,
    private OHOS::IntellVoiceUtils::TaskExecutor {
public:
    ~IntellVoiceServiceManager();
    static std::unique_ptr<IntellVoiceServiceManager> &GetInstance();
    static void SetEnrollResult(IntellVoiceEngineType type, bool result)
    {
        if ((type < INTELL_VOICE_ENROLL) || (type >= ENGINE_TYPE_BUT)) {
            return;
        }

        g_enrollResult[type].store(result);
    }
    static bool GetEnrollResult(IntellVoiceEngineType type)
    {
        if ((type < INTELL_VOICE_ENROLL) || (type >= ENGINE_TYPE_BUT)) {
            return false;
        }

        return g_enrollResult[type].load();
    }

    sptr<IIntellVoiceEngine> HandleCreateEngine(IntellVoiceEngineType type);
    int32_t HandleReleaseEngine(IntellVoiceEngineType type);
    void HandleSilenceUpdate();
    int32_t HandleCloneUpdate(const std::string &wakeupInfo, const sptr<IRemoteObject> &object);
    void HandleSwitchOn(bool isAsync, int32_t uuid, bool needUpdateAdapter);
    void HandleSwitchOff(bool isAsync, int32_t uuid);
    void HandleCloseWakeupSource();
    void HandleUnloadIntellVoiceService(bool isAsync);
    bool HandleOnIdle();
    void HandleServiceStop();
    void HandleHeadsetHostDie();
    void HandlePowerSaveModeChange();

    void ProcBreathModel();
    void CreateSwitchProvider();
    void ReleaseSwitchProvider();
    bool StartDetection(int32_t uuid);
    void StopDetection(int32_t uuid);
    bool QuerySwitchStatus(const std::string &key);

    bool RegisterProxyDeathRecipient(IntellVoiceEngineType type, const sptr<IRemoteObject> &object);
    bool DeregisterProxyDeathRecipient(IntellVoiceEngineType type);

    using UpdateEngineController::OnUpdateComplete;
    using TaskExecutor::AddAsyncTask;
    using TaskExecutor::AddSyncTask;
    using TaskExecutor::StopThread;

    int32_t SetParameter(const std::string &keyValueList);
    std::string GetParameter(const std::string &key);
    int32_t GetWakeupSourceFilesList(std::vector<std::string> &cloneFiles);
    int32_t GetWakeupSourceFile(const std::string &filePath, std::vector<uint8_t> &buffer);
    int32_t SendWakeupFile(const std::string &filePath, const std::vector<uint8_t> &buffer);
    int32_t ClearUserData();
    void SetDspSensibility(const std::string &sensibility);
    void OnTriggerConnectServiceStart();
    void SetScreenOff(bool value);
    bool GetScreenOff();

private:
    IntellVoiceServiceManager();
    static bool IsSwitchKeyValid(const std::string &key)
    {
        if ((key == WAKEUP_KEY) || (key == WHISPER_KEY) || (key == IMPROVE_KEY) || (key == SHORTWORD_KEY)) {
            return true;
        }
        return false;
    }

    bool QuerySwitchByUuid(int32_t uuid)
    {
        if (uuid == VOICE_WAKEUP_MODEL_UUID) {
            return QuerySwitchStatus(WAKEUP_KEY);
        }

        if (uuid == PROXIMAL_WAKEUP_MODEL_UUID) {
            return QuerySwitchStatus(WHISPER_KEY);
        }

        return false;
    }
    sptr<IIntellVoiceEngine> CreateEngine(IntellVoiceEngineType type, const std::string &param = "");
    int32_t ReleaseEngine(IntellVoiceEngineType type);
    void OnSwitchChange(const std::string &switchKey);
    void OnDetected(int32_t uuid);
    void CreateDetector(int32_t uuid);

    sptr<IIntellVoiceEngine> CreateEngineInner(IntellVoiceEngineType type, const std::string &param = "");
    int32_t ReleaseEngineInner(IntellVoiceEngineType type);
    bool CreateOrResetWakeupEngine();
    bool IsEngineExist(IntellVoiceEngineType type);
    int32_t ServiceStopProc();

    void ReleaseUpdateEngine() override;
    bool CreateUpdateEngine(const std::string &param) override;
    void HandleUpdateComplete(UpdateState result, const std::string &param) override;
    void HandleUpdateRetry() override;
    int32_t SilenceUpdate();
    int32_t CloneUpdate(const std::string &wakeupInfo, const sptr<IRemoteObject> &object);
    void RegisterObserver(const std::string &switchKey);
    void SetImproveParam(sptr<EngineBase> engine);
    void CreateAndStartServiceObject(int32_t uuid, bool needResetAdapter);
    void ReleaseServiceObject(int32_t uuid);
    int32_t SwitchOnProc(int32_t uuid, bool needUpdateAdapter);
    int32_t SwitchOffProc(int32_t uuid);
    bool IsNeedToUnloadService();
    int32_t UnloadIntellVoiceService();
    void NoiftySwitchOnToPowerChange();
    bool AddStartDetectionTask(int32_t uuid);
    void DelStartDetectionTask(int32_t uuid);

private:
    static std::unique_ptr<IntellVoiceServiceManager> g_intellVoiceServiceMgr;
    static std::atomic<bool> g_enrollResult[ENGINE_TYPE_BUT];
    std::atomic<bool> screenoff_ = false;
    bool notifyPowerModeChange_ = false;
    std::mutex powerModeChangeMutex_;
    std::condition_variable powerModeChangeCv_;
    std::mutex deathMutex_;
    std::mutex detectorMutex_;
    std::mutex switchMutex_;
    std::map<IntellVoiceEngineType, sptr<EngineBase>> engines_;
    std::map<int32_t, std::shared_ptr<IntellVoiceTrigger::TriggerDetector>> detector_;
    std::map<const std::string, sptr<SwitchObserver>> switchObserver_;
    IntellVoiceUtils::UniqueProductType<SwitchProvider> switchProvider_ =
        IntellVoiceUtils::UniqueProductType<SwitchProvider> {nullptr, nullptr};
    std::map<IntellVoiceEngineType, sptr<IntellVoiceUtils::IntellVoiceDeathRecipient>> proxyDeathRecipient_;
    std::map<IntellVoiceEngineType, sptr<IRemoteObject>> deathRecipientObj_;
    std::map<int32_t, bool> isStarted_;
#ifdef USE_FFRT
    std::shared_ptr<ffrt::queue> taskQueue_ = nullptr;
    std::map<int32_t, ffrt::task_handle> taskHandle_;
#endif
};
}  // namespace IntellVoice
}  // namespace OHOS
#endif
