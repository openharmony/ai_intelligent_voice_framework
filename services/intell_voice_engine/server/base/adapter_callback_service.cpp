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
#include "adapter_callback_service.h"
#include "intell_voice_log.h"

#define LOG_TAG "AdapterCallbackService"

namespace OHOS {
namespace IntellVoiceEngine {
int32_t AdapterCallbackService::OnIntellVoiceHdiEvent(const IntellVoiceEngineCallBackEvent& event)
{
    if (listener_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("listener_ is nullptr");
        return -1;
    }
    INTELL_VOICE_LOG_INFO("send hdi event");
    listener_->OnIntellVoiceHdiEvent(event);
    return 0;
}
}
}