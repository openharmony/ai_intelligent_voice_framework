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
#ifndef SYSTEM_EVENT_OBSERVER_H
#define SYSTEM_EVENT_OBSERVER_H

#include <memory>
#include "common_event_subscriber.h"
#include "event_handler.h"

namespace OHOS {
namespace IntellVoiceEngine {
class SystemEventObserver : public OHOS::EventFwk::CommonEventSubscriber,
    public std::enable_shared_from_this<SystemEventObserver> {
public:
    ~SystemEventObserver();
    static std::shared_ptr<SystemEventObserver> Create(const OHOS::EventFwk::CommonEventSubscribeInfo &subscribeInfo);
    std::shared_ptr<SystemEventObserver> GetPtr()
    {
        return shared_from_this();
    }
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &eventData) override;
    void SetEventHandler(const std::shared_ptr<OHOS::AppExecFwk::EventHandler> &handler);
    bool Subscribe();
    bool Unsubscribe();

private:
    explicit SystemEventObserver(const OHOS::EventFwk::CommonEventSubscribeInfo &subscribeInfo);

    std::shared_ptr<OHOS::AppExecFwk::EventHandler> handler_ = nullptr;
};
}
}
#endif