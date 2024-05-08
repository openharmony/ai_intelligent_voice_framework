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
#ifndef I_HEADSET_WAKEUP_H
#define I_HEADSET_WAKEUP_H

#include <cstdint>
#include <vector>

namespace OHOS {
namespace IntellVoiceEngine {
class IHeadsetWakeup {
public:
    virtual int32_t ReadHeadsetStream(std::vector<uint8_t> &audioStream) = 0;
    virtual int32_t NotifyVerifyResult(bool result) = 0;
    virtual int32_t StopReadingStream() = 0;
    virtual int32_t GetHeadsetAwakeState() = 0;
};
}
}
#endif
