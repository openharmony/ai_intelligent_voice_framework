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
#ifndef UPDATE_ENGINE_H
#define UPDATE_ENGINE_H
#include <memory>
#include <string>
#include "v1_0/iintell_voice_engine_callback.h"
#include "engine_base.h"
#include "engine_util.h"
#include "intell_voice_generic_factory.h"
#include "update_state.h"

namespace OHOS {
namespace IntellVoiceEngine {
class UpdateEngine : public EngineBase, private EngineUtil {
public:
    ~UpdateEngine();
    bool Init(const std::string &param) override;
    void SetCallback(sptr<IRemoteObject> object) override;
    int32_t Attach(const IntellVoiceEngineInfo &info) override;
    int32_t Detach(void) override;
    int32_t Start(bool isLast) override;
    int32_t Stop() override;
    int32_t SetParameter(const std::string &keyValueList) override;
    std::string GetParameter(const std::string &key) override;

private:
    UpdateEngine();
    void OnUpdateEvent(int32_t msgId, int32_t result);
    void OnCommitEnrollComplete(int32_t result);

private:
    std::string name_ = "update engine instance";
    std::string param_;
    UpdateState updateResult_ = UpdateState::UPDATE_STATE_DEFAULT;
    sptr<OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineCallback> callback_ = nullptr;
    std::mutex mutex_;
    friend class IntellVoiceUtils::SptrFactory<EngineBase, UpdateEngine>;
};
}
}
#endif
