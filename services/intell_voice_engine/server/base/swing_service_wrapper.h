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
#ifndef SWING_SERVICE_WRAPPER_H
#define SWING_SERVICE_WRAPPER_H

#include <string>
#include <mutex>

#include "i_swing_service.h"

namespace OHOS {
namespace IntellVoiceEngine {
using GetSwingServiceInstFunc = ISwingService *(*)();

struct SwingServiceManagerPriv {
    void *handle { nullptr };
    GetSwingServiceInstFunc getSwingServiceInst { nullptr };
};

class SwingServiceWrapper {
public:
    SwingServiceWrapper();
    ~SwingServiceWrapper();

    static SwingServiceWrapper &GetInstance()
    {
        static SwingServiceWrapper swingServiceWrapper;
        return swingServiceWrapper;
    }

    int32_t SubscribeSwingEvent(std::string swingEventType, std::map<std::string, std::string> eventParams);
    int32_t UnSubscribeSwingEvent(std::string swingEventType, std::map<std::string, std::string> eventParams);

private:
    int32_t LoadSwingServiceLib();
    void UnloadSwingServiceLib();

private:
    std::mutex mutex_ {};
    SwingServiceManagerPriv swingServicePriv_;
    ISwingService *inst_ = nullptr;
};
}
}
#endif