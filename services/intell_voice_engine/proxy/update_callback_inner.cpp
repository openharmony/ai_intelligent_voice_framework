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

#include "update_callback_inner.h"
#include "intell_voice_log.h"

using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;
#define LOG_TAG "UpdateCallbackInner"

namespace OHOS {
namespace IntellVoice {
void UpdateCallbackInner::OnUpdateComplete(int result)
{
    INTELL_VOICE_LOG_INFO("receive result");
    if (cb_ == nullptr) {
        INTELL_VOICE_LOG_INFO("cb is null");
        return;
    }
    cb_->OnUpdateComplete(result);
}

void UpdateCallbackInner::SetUpdateCallback(std::shared_ptr<IIntellVoiceUpdateCallback> cb)
{
    cb_ = cb;
}
}
}