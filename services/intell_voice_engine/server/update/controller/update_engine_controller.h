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
#include "intell_voice_generic_factory.h"
#include "timer_mgr.h"
#include "update_state.h"
#include "update_strategy.h"

namespace OHOS {
namespace IntellVoiceEngine {
class UpdateEngineController : public OHOS::IntellVoiceUtils::ITimerObserver,
    private OHOS::IntellVoiceUtils::TimerMgr {
public:
    virtual ~UpdateEngineController();
    UpdateEngineController();

    virtual bool CreateUpdateEngine(const std::string &param)
    {
        return false;
    }
    virtual void ReleaseUpdateEngine() {};
    void OnUpdateComplete(UpdateState result, const std::string &param);
    int CreateUpdateEngineUntilTime(std::shared_ptr<IUpdateStrategy> updateStrategy);

    static bool GetUpdateState()
    {
        return isUpdating_.load();
    }

protected:
    void UpdateCompleteProc(UpdateState result, const std::string &param, bool &isLast);
    bool UpdateRetryProc();

private:
    virtual void HandleUpdateComplete(UpdateState result, const std::string &param) {};
    virtual void HandleUpdateRetry() {};
    void OnTimerEvent(OHOS::IntellVoiceUtils::TimerEvent &info) override;
    void StartUpdateTimer();
    void StopUpdateTimer();
    bool IsNeedRetryUpdate();
    static void SetUpdateState(bool state)
    {
        isUpdating_.store(state);
    }
    void ClearRetryState(void);
    int UpdateArbitration(int priority);
private:
    static std::atomic<bool> isUpdating_;
    int timerId_ = OHOS::IntellVoiceUtils::INVALID_ID;
    int retryTimes_ = 0;
    int retryTimesLimit_ = 0;
    int delaySecond_ = UPDATE_DELAY_TIME_SECONDS;
    std::mutex updateEngineMutex_;
    UpdateState updateResult_ = UpdateState::UPDATE_STATE_DEFAULT;
    std::shared_ptr<IUpdateStrategy> updateStrategy_;
    int curPriority_ = UPDATE_PRIORITY_DEFAULT;
};
}
}
#endif
