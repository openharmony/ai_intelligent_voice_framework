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

#ifndef IINTELL_VOICE_SERVICE_H
#define IINTELL_VOICE_SERVICE_H
#include "iremote_broker.h"
#include "v1_2/intell_voice_engine_types.h"
#include "i_intell_voice_engine.h"

#include "i_intell_voice_update_callback.h"

namespace OHOS {
namespace IntellVoiceEngine {
using OHOS::HDI::IntelligentVoice::Engine::V1_2::UploadHdiFile;

class IIntellVoiceService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IntellVoiceFramework.Service");

    enum {
        HDI_INTELL_VOICE_SERVICE_CREATE_ENGINE = 0,
        HDI_INTELL_VOICE_SERVICE_RELEASE_ENGINE,
        HDI_INTELL_VOICE_SERVICE_GET_REPORTED_FILES,
        HDI_INTELL_VOICE_SERVICE_CREATE_EVALUATOR,
        HDI_INTELL_VOICE_SERVICE_RELEASE_EVALUATOR,
        HDI_INTELL_VOICE_SERVICE_GET_PARAMETER,
        HDI_INTELL_VOICE_SERVICE_GET_CLONE_FILES_LIST,
        HDI_INTELL_VOICE_SERVICE_GET_CLONE_FILE,
        HDI_INTELL_VOICE_SERVICE_SEND_CLONE_FILE,
        HDI_INTELL_VOICE_SERVICE_CLONE_FOR_RESULT
    };

    virtual int32_t CreateIntellVoiceEngine(IntellVoiceEngineType type, sptr<IIntellVoiceEngine> &inst) = 0;
    virtual int32_t ReleaseIntellVoiceEngine(IntellVoiceEngineType type) = 0;
    virtual int32_t GetUploadFiles(int numMax, std::vector<UploadHdiFile> &files) = 0;
    virtual std::string GetParameter(const std::string &key) = 0;
    virtual int32_t GetWakeupSourceFilesList(std::vector<std::string>& cloneFiles) = 0;
    virtual int32_t GetWakeupSourceFile(const std::string &filePath, std::vector<uint8_t> &buffer) = 0;
    virtual int32_t SendWakeupFile(const std::string &filePath, const std::vector<uint8_t> &buffer) = 0;
    virtual int32_t EnrollWithWakeupFilesForResult(const std::string &wakeupInfo,
        const sptr<IRemoteObject> object) = 0;
};
}
}
#endif
