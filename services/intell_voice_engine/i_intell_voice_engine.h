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

#ifndef IINTELL_VOICE_ENGINE_H
#define IINTELL_VOICE_ENGINE_H
#include "iremote_broker.h"
#include "i_intell_voice_engine_callback.h"

namespace OHOS {
namespace IntellVoiceEngine {
enum IntellVoiceEngineType {
    INTELL_VOICE_ENROLL = 0,
    INTELL_VOICE_WAKEUP,
    INTELL_VOICE_UPDATE,
    ENGINE_TYPE_BUT
};

struct IntellVoiceEngineInfo {
    std::string wakeupPhrase;
    bool isPcmFromExternal { false };
    int32_t minBufSize { 0 };
    int32_t sampleChannels { 0 };
    int32_t bitsPerSample { 0 };
    int32_t sampleRate { 0 };
};

class IIntellVoiceEngine : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"HDI.IntellVoice.Engine");

    enum {
        INTELL_VOICE_ENGINE_SET_CALLBACK = 0,
        INTELL_VOICE_ENGINE_ATTACH,
        INTELL_VOICE_ENGINE_DETACH,
        INTELL_VOICE_ENGINE_SET_PARAMETER,
        INTELL_VOICE_ENGINE_GET_PARAMETER,
        INTELL_VOICE_ENGINE_START,
        INTELL_VOICE_ENGINE_STOP,
        INTELL_VOICE_ENGINE_WRITE_AUDIO,
        INTELL_VOICE_ENGINE_READ,
        INTELL_VOICE_ENGINE_STAET_CAPTURER,
        INTELL_VOICE_ENGINE_STOP_CAPTURER
    };

    virtual void SetCallback(sptr<IRemoteObject> object) = 0;
    virtual int32_t Attach(const IntellVoiceEngineInfo &info) = 0;
    virtual int32_t Detach(void) = 0;
    virtual int32_t SetParameter(const std::string &keyValueList) = 0;
    virtual std::string GetParameter(const std::string &key) = 0;
    virtual int32_t Start(bool isLast) = 0;
    virtual int32_t Stop(void) = 0;
    virtual int32_t WriteAudio(const uint8_t *buffer, uint32_t size) = 0;
    virtual int32_t Read(std::vector<uint8_t> &data) = 0;
    virtual int32_t StartCapturer(int32_t channels) = 0;
    virtual int32_t StopCapturer() = 0;
};
}
}

#endif
