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

#ifndef INTELL_VOICE_SERVICE_PROXY_H
#define INTELL_VOICE_SERVICE_PROXY_H

#include <iremote_proxy.h>
#include "i_intell_voice_service.h"
#include "i_intell_voice_engine.h"

namespace OHOS {
namespace IntellVoiceEngine {
class IntellVoiceServiceProxy : public IRemoteProxy<IIntellVoiceService> {
public:
    explicit IntellVoiceServiceProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IIntellVoiceService>(impl) {}
    virtual ~IntellVoiceServiceProxy() {};

    int32_t CreateIntellVoiceEngine(IntellVoiceEngineType type, sptr<IIntellVoiceEngine> &inst) override;
    int32_t ReleaseIntellVoiceEngine(IntellVoiceEngineType type) override;
    int32_t GetUploadFiles(int numMax, std::vector<UploadHdiFile> &files) override;
    std::string GetParameter(const std::string &key) override;
    int32_t GetWakeupSourceFilesList(std::vector<std::string> &cloneFiles) override;
    int32_t GetWakeupSourceFile(const std::string &filePath, std::vector<uint8_t> &buffer) override;
    int32_t SendWakeupFile(const std::string &filePath, const std::vector<uint8_t> &buffer) override;
    int32_t EnrollWithWakeupFilesForResult(const std::string &wakeupInfo, const sptr<IRemoteObject> object) override;

private:
    static inline BrokerDelegator<IntellVoiceServiceProxy> delegator_;
};
}
}

#endif
