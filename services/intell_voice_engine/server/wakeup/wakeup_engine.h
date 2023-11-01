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
#ifndef WAKEUP_ENGINE_H
#define WAKEUP_ENGINE_H
#include <memory>
#include <string>
#include "engine_base.h"
#include "intell_voice_engine_stub.h"
#include "wakeup_adapter_listener.h"
#include "wakeup_source_stop_callback.h"
#include "v1_0/iintell_voice_engine_callback.h"

#include "audio_info.h"
#include "audio_source.h"
#include "intell_voice_generic_factory.h"

namespace OHOS {
namespace IntellVoiceEngine {
class WakeupEngine : public EngineBase {
public:
    ~WakeupEngine();
    bool Init() override;
    void SetCallback(sptr<IRemoteObject> object) override;
    int32_t Attach(const IntellVoiceEngineInfo &info) override;
    int32_t Detach(void) override;
    int32_t Start(bool isLast) override;
    int32_t Stop() override;

    void OnDetected() override;
    bool ResetAdapter() override;
    void ReleaseAdapter() override;

private:
    WakeupEngine();
    void OnWakeupEvent(int32_t msgId, int32_t result);

    void OnWakeupRecognition();
    bool SetCallback();
    bool StartAudioSource();
    void StartAbility();
    void StopAudioSource();
    bool CreateWakeupSourceStopCallback();

private:
    bool isPcmFromExternal_ = false;
    std::shared_ptr<WakeupAdapterListener> adapterListener_ = nullptr;
    sptr<OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineCallback> callback_ = nullptr;
    std::shared_ptr<WakeupSourceStopCallback> wakeupSourceStopCallback_ = nullptr;
    OHOS::AudioStandard::AudioCapturerOptions capturerOptions_;
    std::unique_ptr<AudioSource> audioSource_ = nullptr;
    friend class IntellVoiceUtils::SptrFactory<WakeupEngine>;
};
}
}
#endif