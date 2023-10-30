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
#ifndef ENROLL_ENGINE_H
#define ENROLL_ENGINE_H
#include <memory>
#include <string>
#include "engine_base.h"
#include "v1_0/iintell_voice_engine_callback.h"
#include "audio_info.h"
#include "audio_source.h"
#include "intell_voice_generic_factory.h"

namespace OHOS {
namespace IntellVoiceEngine {
class EnrollEngine : public EngineBase {
public:
    ~EnrollEngine();
    virtual bool Init() override;
    void SetCallback(sptr<IRemoteObject> object) override;
    int32_t Attach(const IntellVoiceEngineInfo &info) override;
    int32_t Detach(void) override;
    int32_t Start(bool isLast) override;
    int32_t Stop() override;
    int32_t SetParameter(const std::string &keyValueList) override;

private:
    EnrollEngine();
    bool SetParameterInner(const std::string &keyValueList);
    bool StartAudioSource();
    void StopAudioSource();
    void OnEnrollEvent(int32_t msgId, int32_t result);
    void OnEnrollComplete();

private:
    std::string name_ = "lp enroll engine instance";
    bool isPcmFromExternal_ = false;
    int32_t enrollResult_ = -1;
    sptr<OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineCallback> callback_ = nullptr;
    OHOS::AudioStandard::AudioCapturerOptions capturerOptions_;
    std::unique_ptr<AudioSource> audioSource_ = nullptr;
    friend class IntellVoiceUtils::SptrFactory<EnrollEngine>;
};
}
}
#endif
