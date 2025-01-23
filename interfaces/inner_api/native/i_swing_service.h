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
#ifndef I_SWING_SERVICE_H
#define I_SWING_SERVICE_H

#include <map>
#include <cstdint>
#include <vector>

namespace OHOS {
namespace IntellVoiceEngine {
class ISwingService {
public:
    virtual ~ISwingService() = default;
    virtual int32_t SubscribeSwingEvent(std::string swingEventType,
        std::map<std::string, std::string> eventParams) = 0;
    virtual int32_t UnSubscribeSwingEvent(std::string swingEventType,
        std::map<std::string, std::string> eventParams) = 0;
};
}
}
#endif
