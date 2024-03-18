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

namespace OHOS {
namespace IntellVoiceEngine {
constexpr int32_t VOICE_WAKEUP_MODEL_UUID = 1;
constexpr int32_t PROXIMAL_WAKEUP_MODEL_UUID = 2;
const std::string WAKEUP_KEY = "intell_voice_trigger_enabled";
const std::string BREATH_KEY = "intell_voice_breath_enabled";
const std::string IMPROVE_KEY = "intell_voice_improve_enabled";
const std::string SHORTWORD_KEY = "intell_voice_shortword_enabled";

class IntellVoiceServiceManager : private IntellVoiceEngineArbitration, private UpdateEngineController {
public:
    ~IntellVoiceServiceManager();
    static std::unique_ptr<IntellVoiceServiceManager> &GetInstance();
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
    sptr<IIntellVoiceEngine> CreateEngine(IntellVoiceEngineType type, const std::string &param = "");
    int32_t ReleaseEngine(IntellVoiceEngineType type);

    void OnServiceStart();
    void OnServiceStop();
    void CreateSwitchProvider();
    void ReleaseSwitchProvider();
    void StartDetection(int32_t uuid);
    void StopDetection();
    bool QuerySwitchStatus(const std::string &key);
    void UnloadIntellVoiceService();

    bool RegisterProxyDeathRecipient(IntellVoiceEngineType type, const sptr<IRemoteObject> &object);
    bool DeregisterProxyDeathRecipient(IntellVoiceEngineType type);

    using UpdateEngineController::OnUpdateComplete;

    std::string GetParameter(const std::string &key);
    int32_t GetWakeupSourceFilesList(std::vector<std::string>& cloneFiles);
    int32_t GetWakeupSourceFile(const std::string &filePath, std::vector<uint8_t> &buffer);
    int32_t SendWakeupFile(const std::string &filePath, const std::vector<uint8_t> &buffer);
    int32_t CloneUpdate(const std::string &wakeupInfo, const sptr<IRemoteObject> &object);
    int32_t SilenceUpdate();
private:
    IntellVoiceServiceManager();
    void OnWakeupSwitchChange();
    void OnSwitchChange(const std::string &switchKey);
    void OnDetected(int32_t uuid);
    void CreateDetector(int32_t uuid);

    sptr<IIntellVoiceEngine> CreateEngineInner(IntellVoiceEngineType type, const std::string &param = "");
    int32_t ReleaseEngineInner(IntellVoiceEngineType type);
    bool CreateOrResetWakeupEngine();
    bool IsEngineExist(IntellVoiceEngineType type);

    void ReleaseUpdateEngine() override;
    bool CreateUpdateEngine(const std::string &param) override;
    void UpdateCompleteHandler(UpdateState result, bool islast) override;
    void RegisterObserver(const std::string &switchKey);
    void SetImproveParam(sptr<EngineBase> engine);
    void ProcBreathModel();
    void CreateAndStartServiceObject(int32_t uuid);

private:
    static std::unique_ptr<IntellVoiceServiceManager> g_intellVoiceServiceMgr;
    static std::atomic<bool> enrollResult_[ENGINE_TYPE_BUT];
    std::atomic<bool> isServiceUnloaded_ = false;
    std::mutex engineMutex_;
    std::mutex detectorMutex_;
    std::mutex switchMutex_;
    std::map<IntellVoiceEngineType, sptr<EngineBase>> engines_;
    std::map<int32_t, std::shared_ptr<IntellVoiceTrigger::TriggerDetector>> detector_;
    std::map<const std::string, sptr<SwitchObserver>> switchObserver_;
    IntellVoiceUtils::UniqueProductType<SwitchProvider> switchProvider_ =
        IntellVoiceUtils::UniqueProductType<SwitchProvider> {nullptr, nullptr};
    std::map<IntellVoiceEngineType, sptr<IntellVoiceUtils::IntellVoiceDeathRecipient>> proxyDeathRecipient_;
    std::map<IntellVoiceEngineType, sptr<IRemoteObject>> deathRecipientObj_;
};
}  // namespace IntellVoice
}  // namespace OHOS
#endif
