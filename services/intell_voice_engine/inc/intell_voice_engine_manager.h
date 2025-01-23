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
#ifndef ENGINE_MANAGER_H
#define ENGINE_MANAGER_H
#include <mutex>
#include <map>
#include <atomic>
#include "engine_base.h"
#include "intell_voice_engine_arbitration.h"
#include "intell_voice_death_recipient.h"
#include "update_engine_controller.h"
#include "task_executor.h"
#include "intell_voice_definitions.h"
#include "i_intell_voice_service.h"

namespace OHOS {
namespace IntellVoiceEngine {
class IntellVoiceEngineManager : private IntellVoiceEngineArbitration,
    private UpdateEngineController {
public:
    IntellVoiceEngineManager();
    ~IntellVoiceEngineManager();
    static std::shared_ptr<IntellVoiceEngineManager> GetInstance();
    static bool GetScreenOff();
    void SetScreenOff(bool value);
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

    bool AnyEngineExist(const std::vector<IntellVoiceEngineType> &types);
    bool RegisterProxyDeathRecipient(IntellVoiceEngineType type, const sptr<IRemoteObject> &object);
    bool DeregisterProxyDeathRecipient(IntellVoiceEngineType type);

    int32_t GetUploadFiles(int numMax, std::vector<UploadFilesFromHdi> &files);
    int32_t SetParameter(const std::string &sensibility);
    std::string GetParameter(const std::string &key);
    int32_t GetWakeupSourceFilesList(std::vector<std::string> &cloneFiles);
    int32_t GetWakeupSourceFile(const std::string &filePath, std::vector<uint8_t> &buffer);
    int32_t SendWakeupFile(const std::string &filePath, const std::vector<uint8_t> &buffer);
    std::string GetDspSensibility(const std::string &sensibility, const std::string &dspFeature,
    const std::string &configPath);

    void ClearUserDataInner();
    void HeadsetHostDie();
    bool IsNeedUpdateComplete(int32_t result, const std::string &param);
    bool IsNeedUpdateRetry();
    void EngineOnDetected(int32_t uuid);
    void ImproveKeySwitch();
    void ClearWakeupEngineCb();
    bool CreateOrResetWakeupEngine();
    bool IsEngineExist(IntellVoiceEngineType type);
    sptr<IIntellVoiceEngine> CreateEngine(IntellVoiceEngineType type, const std::string &param = "");
    int32_t ReleaseEngineInner(IntellVoiceEngineType type);
    int32_t SilenceUpdate();
    int32_t CloneUpdate(const std::string &wakeupInfo, const sptr<IRemoteObject> &object);
    int32_t ServiceStopProc();
    void SetDspSensibility(const std::string &sensibility);
    void OnServiceStart();
    void OnServiceStop();

private:

    sptr<IIntellVoiceEngine> CreateEngineInner(IntellVoiceEngineType type, const std::string &param = "");
    void ReleaseUpdateEngine() override;
    bool CreateUpdateEngine(const std::string &param) override;
    void SetImproveParam(sptr<EngineBase> engine);
    void LoadIntellVoiceHost();
    void UnloadIntellVoiceHost();

private:
    static std::mutex instanceMutex_;
    static std::shared_ptr<IntellVoiceEngineManager> instance_;
    static std::atomic<bool> g_enrollResult[ENGINE_TYPE_BUT];
    static std::atomic<bool> screenoff_;
    std::mutex deathMutex_;
    std::map<IntellVoiceEngineType, sptr<EngineBase>> engines_;
    std::map<IntellVoiceEngineType, sptr<IntellVoiceUtils::IntellVoiceDeathRecipient>> proxyDeathRecipient_;
    std::map<IntellVoiceEngineType, sptr<IRemoteObject>> deathRecipientObj_;
};
}  // namespace IntellVoice
}  // namespace OHOS
#endif
