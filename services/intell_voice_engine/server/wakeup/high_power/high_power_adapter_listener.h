/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef HIGH_POWER_ADAPTER_LISTENER_H
#define HIGH_POWER_ADAPTER_LISTENER_H

#include <functional>
#include <memory>
#include <mutex>
#include "time_util.h"
#include "intell_voice_adapter_listener.h"
#include "i_intell_voice_engine_callback.h"

namespace OHOS {
namespace IntellVoiceEngine {

class HighPowerAdapterListener : public IntellVoiceAdapterListener {
public:
    explicit HighPowerAdapterListener();
    ~HighPowerAdapterListener();
    void SetCallback(const sptr<IIntelligentVoiceEngineCallback> &cb);
    std::string GetCallbackStatus();
    void OnIntellVoiceHdiEvent(
        const OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineCallBackEvent &event) override;
private:
    void BackupCallBackEvent(const OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineCallBackEvent &event);

private:
    std::mutex mutex_;
    sptr<IIntelligentVoiceEngineCallback> cb_ = nullptr;
    std::shared_ptr<OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineCallBackEvent> historyEvent_ = nullptr;
    uint64_t historyMsgTime_;
};
}  // namespace IntellVoice
}  // namespace OHOS
#endif