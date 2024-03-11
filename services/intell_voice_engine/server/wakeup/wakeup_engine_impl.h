/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef WAKEUP_ENGINE_IMPL_H
#define WAKEUP_ENGINE_IMPL_H

#include <memory>
#include <string>
#include "v1_0/iintell_voice_engine_callback.h"
#include "audio_info.h"
#include "i_intell_voice_engine.h"
#include "state_manager.h"
#include "engine_util.h"
#include "wakeup_adapter_listener.h"
#include "wakeup_source_stop_callback.h"

#include "audio_source.h"
#include "wakeup_source_process.h"

namespace OHOS {
namespace IntellVoiceEngine {
using OHOS::IntellVoiceUtils::StateMsg;
using OHOS::IntellVoiceUtils::State;

enum EngineEvent {
    NONE = 0,
    INIT,
    INIT_DONE,
    START_RECOGNIZE,
    STOP_RECOGNIZE,
    RECOGNIZE_COMPLETE,
    START_CAPTURER,
    READ,
    STOP_CAPTURER,
    RECOGNIZING_TIMEOUT,
    RECOGNIZE_COMPLETE_TIMEOUT,
    READ_CAPTURER_TIMEOUT,
    SET_LISTENER,
    SET_PARAM,
    GET_PARAM,
    GET_WAKEUP_PCM,
    RELEASE_ADAPTER,
    RESET_ADAPTER,
    RELEASE,
};

struct SetListenerMsg {
    explicit SetListenerMsg(sptr<IIntelligentVoiceEngineCallback> cb) : callback(cb) {}
    sptr<IIntelligentVoiceEngineCallback> callback = nullptr;
};

struct CapturerData {
    std::vector<uint8_t> data;
};

struct StringParam {
    explicit StringParam(const std::string &str = "") : strParam(str) {}
    std::string strParam;
};

class WakeupEngineImpl : private OHOS::IntellVoiceUtils::ModuleStates, private EngineUtil,
    private WakeupSourceProcess {
public:
    WakeupEngineImpl();
    ~WakeupEngineImpl();
    int32_t Handle(const StateMsg &msg);

private:
    enum EngineState {
        IDLE = 0,
        INITIALIZING = 1,
        INITIALIZED = 2,
        RECOGNIZING = 3,
        RECOGNIZED = 4,
        READ_CAPTURER = 5,
    };

private:
    bool SetCallbackInner();
    int32_t AttachInner(const IntellVoiceEngineInfo &info);
    bool StartAudioSource();
    void StopAudioSource();
    bool CreateWakeupSourceStopCallback();
    void DestroyWakeupSourceStopCallback();
    void OnWakeupEvent(int32_t msgId, int32_t result);
    void OnInitDone(int32_t result);
    void OnWakeupRecognition(int32_t result);
    OHOS::AudioStandard::AudioChannel GetWakeupSourceChannel();

private:
    bool InitStates();
    int32_t HandleGetParam(const StateMsg &msg, State &nextState);
    int32_t HandleSetParam(const StateMsg &msg, State &nextState);
    int32_t HandleInit(const StateMsg &msg, State &nextState);
    int32_t HandleSetListener(const StateMsg &msg, State &nextState);
    int32_t HandleInitDone(const StateMsg &msg, State &nextState);
    int32_t HandleStart(const StateMsg &msg, State &nextState);
    int32_t HandleStop(const StateMsg &msg, State &nextState);
    int32_t HandleRecognizeComplete(const StateMsg &msg, State &nextState);
    int32_t HandleStartCapturer(const StateMsg &msg, State &nextState);
    int32_t HandleRead(const StateMsg &msg, State &nextState);
    int32_t HandleStopCapturer(const StateMsg &msg, State &nextState);
    int32_t HandleGetWakeupPcm(const StateMsg &msg, State &nextState);
    int32_t HandleRecognizingTimeout(const StateMsg &msg, State &nextState);
    int32_t HandleResetAdapter(const StateMsg &msg, State &nextState);
    int32_t HandleRelease(const StateMsg &msg, State &nextState);

private:
    using EngineUtil::adapter_;
    bool isPcmFromExternal_ = false;
    int32_t channels_ = 0;
    sptr<OHOS::HDI::IntelligentVoice::Engine::V1_0::IIntellVoiceEngineCallback> callback_ = nullptr;
    std::shared_ptr<WakeupAdapterListener> adapterListener_ = nullptr;
    std::shared_ptr<WakeupSourceStopCallback> wakeupSourceStopCallback_ = nullptr;
    std::unique_ptr<AudioSource> audioSource_ = nullptr;
    OHOS::AudioStandard::AudioCapturerOptions capturerOptions_;
};
}
}
#endif