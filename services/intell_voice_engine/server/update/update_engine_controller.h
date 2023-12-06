/*
 * Copyright (c) 2023 Huawei Device Co., Ltd. 2023-2023.
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
#ifndef UPDATE_ENGINE_CONTROLLER_H
#define UPDATE_ENGINE_CONTROLLER_H
#include <memory>
#include <string>
#include <atomic>
#include "engine_base.h"
#include "v1_0/iintell_voice_engine_callback.h"
#include "intell_voice_generic_factory.h"
#include "timer_mgr.h"
#include "update_state.h"

namespace OHOS {
namespace IntellVoiceEngine {
class UpdateEngineController : private OHOS::IntellVoiceUtils::ITimerObserver,
    private OHOS::IntellVoiceUtils::TimerMgr {
public:
    virtual ~UpdateEngineController();
    UpdateEngineController();

    bool IsVersionUpdate();
    void SaveWakeupVesion();
    void GetCurWakeupVesion(std::string &versionNumber);
    virtual bool CreateUpdateEngine()
    {
        return false;
    }
    virtual void ReleaseUpdateEngine() {};
    virtual void UpdateCompleteHandler(UpdateState result, bool islast) {};
    void OnUpdateComplete(UpdateState result);
    bool CreateUpdateEngineUntilTime(int delaySecond = 0);

    static bool GetUpdateState()
    {
        return isUpdating_.load();
    }

private:
    void OnTimerEvent(OHOS::IntellVoiceUtils::TimerEvent& info) override;
    void OnUpdateRetry();
    void StartUpdateTimer();
    void StopUpdateTimer();
    bool IsNeedRetryUpdate();
    static void SetUpdateState(bool state)
    {
        isUpdating_.store(state);
    }
    void ClearRetryState(void);
private:
    static std::atomic<bool> isUpdating_;
    int timerId_ = OHOS::IntellVoiceUtils::INVALID_ID;
    int retryTimes = 0;
    int delaySecond_;
    std::mutex updateEngineMutex_;
    UpdateState updateResult_ = UpdateState::UPDATE_STATE_DEFAULT;
};
}
}
#endif
