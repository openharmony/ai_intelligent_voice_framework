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
#include "system_event_observer.h"
#include "event_handler.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "intell_voice_log.h"
#include "intell_voice_service_manager.h"

#define LOG_TAG "SystemEventObserver"

using namespace OHOS::AppExecFwk;
using namespace OHOS::EventFwk;

namespace OHOS {
namespace IntellVoiceEngine {
SystemEventObserver::SystemEventObserver(const OHOS::EventFwk::CommonEventSubscribeInfo &subscribeInfo,
    SystemEventReceiver receiver) : EventFwk::CommonEventSubscriber(subscribeInfo), receiver_(receiver)
{
    INTELL_VOICE_LOG_INFO("SystemEventObserver create");
}

SystemEventObserver::~SystemEventObserver()
{
}

std::shared_ptr<SystemEventObserver> SystemEventObserver::Create(
    const OHOS::EventFwk::CommonEventSubscribeInfo &subscribeInfo, SystemEventReceiver receiver)
{
    return std::shared_ptr<SystemEventObserver>(new (std::nothrow) SystemEventObserver(subscribeInfo, receiver));
}

bool SystemEventObserver::Subscribe()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ != IDLE) {
        INTELL_VOICE_LOG_INFO("state(%{public}d) is not idle", state_);
        return true;
    }
    if (!OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(GetPtr())) {
        INTELL_VOICE_LOG_ERROR("SubscribeCommonEvent occur exception.");
        return false;
    }
    state_ = SUBSCRIBED;
    return true;
}

bool SystemEventObserver::Unsubscribe()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ != SUBSCRIBED) {
        INTELL_VOICE_LOG_INFO("state(%{public}d) is not subscribe", state_);
        return true;
    }
    if (!OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(GetPtr())) {
        INTELL_VOICE_LOG_ERROR("UnsubscribeCommonEvent occur exception.");
    }
    state_ = UNSUBSCRIBED;
    return true;
}

void SystemEventObserver::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &eventData)
{
    INTELL_VOICE_LOG_INFO("enter");

    const OHOS::AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        INTELL_VOICE_LOG_INFO("COMMON_EVENT_SCREEN_ON");
        auto &mgr = IntellVoiceServiceManager::GetInstance();
        if (mgr != nullptr) {
            mgr->SetScreenOff(false);
        }
    } else if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        INTELL_VOICE_LOG_INFO("COMMON_EVENT_SCREEN_OFF");
        auto &mgr = IntellVoiceServiceManager::GetInstance();
        if (mgr != nullptr) {
            mgr->SetScreenOff(true);
        }
    } else if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        INTELL_VOICE_LOG_INFO("COMMON_EVENT_PACKAGE_REMOVED");
    } else if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REPLACED) {
        INTELL_VOICE_LOG_INFO("COMMON_EVENT_PACKAGE_REPLACED");
    } else if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED) {
        INTELL_VOICE_LOG_INFO("COMMON_EVENT_PACKAGE_DATA_CLEARED");
    } else if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_DATA_SHARE_READY) {
        INTELL_VOICE_LOG_INFO("COMMON_EVENT_DATA_SHARE_READY");
        if (receiver_ != nullptr) {
            receiver_(eventData);
        }
    } else {
        INTELL_VOICE_LOG_INFO("unkonw event, action:%{public}s", action.c_str());
    }
}
}
}