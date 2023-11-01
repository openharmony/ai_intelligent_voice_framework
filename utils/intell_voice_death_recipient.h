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

#ifndef INTELL_VOICE_DEATH_RECIPIENT_H
#define INTELL_VOICE_DEATH_RECIPIENT_H

#include "iremote_object.h"
#include "nocopyable.h"

namespace OHOS {
namespace IntellVoiceUtils {
class IntellVoiceDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    using ServerDiedCallback = std::function<void()>;
    explicit IntellVoiceDeathRecipient(ServerDiedCallback callback) : callback_(callback) {};
    ~IntellVoiceDeathRecipient() override = default;

    void OnRemoteDied(const wptr<IRemoteObject> &remote) override
    {
        (void)remote;
        if (callback_ != nullptr) {
            callback_();
        }
    }

private:
    ServerDiedCallback callback_ = nullptr;
    DISALLOW_COPY_AND_MOVE(IntellVoiceDeathRecipient);
};
}  // namespace IntellVoiceEngine
}  // namespace OHOS

#endif  // AUDIO_STREAM_DEATH_RECIPIENT_H
