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
#include "wakeup_adapter_listener.h"

#include <thread>
#include "intell_voice_log.h"
#include "v1_2/intell_voice_engine_types.h"

#define LOG_TAG "WakeupAdapterListener"

using namespace OHOS::HDI::IntelligentVoice::Engine::V1_2;

namespace OHOS {
namespace IntellVoiceEngine {
WakeupAdapterListener::WakeupAdapterListener(OnWakeupEventCb wakeupEventCb) : wakeupEventCb_(wakeupEventCb)
{
    INTELL_VOICE_LOG_INFO("constructor");
}

WakeupAdapterListener::~WakeupAdapterListener()
{
    INTELL_VOICE_LOG_INFO("destructor");
}

void WakeupAdapterListener::SetCallback(const sptr<IIntelligentVoiceEngineCallback> &cb)
{
    INTELL_VOICE_LOG_INFO("enter");
    std::lock_guard<std::mutex> lock(mutex_);
    if (cb == nullptr) {
        INTELL_VOICE_LOG_INFO("clear callback");
        cb_ = nullptr;
        return;
    }
    cb_ = cb;
    if (historyEvent_ != nullptr) {
        cb_->OnIntellVoiceEngineEvent(*(historyEvent_.get()));
        historyEvent_ = nullptr;
    }
}

void WakeupAdapterListener::OnIntellVoiceHdiEvent(const IntellVoiceEngineCallBackEvent &event)
{
    INTELL_VOICE_LOG_INFO("OnIntellVoiceHdiEvent");

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (cb_ == nullptr) {
            INTELL_VOICE_LOG_WARN("cb_ is nullptr");
            BackupCallBackEvent(event);
            wakeupEventCb_(event.msgId, event.result);
            return;
        }

        historyEvent_ = nullptr;
        if ((event.msgId == OHOS::HDI::IntelligentVoice::Engine::V1_0::INTELL_VOICE_ENGINE_MSG_RECOGNIZE_COMPLETE) ||
            (event.msgId == static_cast<OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineMessageType>(
                INTELL_VOICE_ENGINE_MSG_RECONFIRM_RECOGNITION_COMPLETE)) || (
                event.msgId == static_cast<OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineMessageType>(
                INTELL_VOICE_ENGINE_MSG_HEADSET_RECOGNIZE_COMPLETE))) {
            cb_->OnIntellVoiceEngineEvent(event);
        }
    }

    wakeupEventCb_(event.msgId, event.result);
}

void WakeupAdapterListener::BackupCallBackEvent(const IntellVoiceEngineCallBackEvent &event)
{
    if ((event.msgId == OHOS::HDI::IntelligentVoice::Engine::V1_0::INTELL_VOICE_ENGINE_MSG_RECOGNIZE_COMPLETE) || (
        event.msgId == static_cast<OHOS::HDI::IntelligentVoice::Engine::V1_0::IntellVoiceEngineMessageType>(
        INTELL_VOICE_ENGINE_MSG_HEADSET_RECOGNIZE_COMPLETE))) {
        INTELL_VOICE_LOG_INFO("backup callBackEvent, msg id:%{public}d", event.msgId);

        historyEvent_ = std::make_shared<IntellVoiceEngineCallBackEvent>();
        if (historyEvent_ == nullptr) {
            INTELL_VOICE_LOG_INFO("historyEvent_ is nullptr");
            return;
        }

        historyEvent_->msgId = event.msgId;
        historyEvent_->result = event.result;
        historyEvent_->info = event.info;
    } else {
        INTELL_VOICE_LOG_WARN("unknow msg id:%{public}d", event.msgId);
    }
}
}
}