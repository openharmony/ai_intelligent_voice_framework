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
#ifndef INTELL_VOICE_TRIGGER_CONNECTOR_MGR_H
#define INTELL_VOICE_TRIGGER_CONNECTOR_MGR_H

#include "nocopyable.h"
#include "i_intell_voice_trigger_connector_internal.h"

namespace OHOS {
namespace IntellVoiceTrigger {
class TriggerConnectorMgr {
public:
    ~TriggerConnectorMgr() = default;
    static std::unique_ptr<TriggerConnectorMgr> &GetInstance();
    std::vector<TriggerConnectorModuleDesc> ListConnectorModuleDescriptors();
    std::shared_ptr<IIntellVoiceTriggerConnectorModule> GetConnectorModule(
        const std::string &adapterName, std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback);

private:
    explicit TriggerConnectorMgr(std::unique_ptr<IIntellVoiceTriggerConnectorInternal> delegate);

private:
    static std::unique_ptr<TriggerConnectorMgr> g_connectorMgr;
    std::unique_ptr<IIntellVoiceTriggerConnectorInternal> delegate_ = nullptr;

    DISALLOW_COPY(TriggerConnectorMgr);
    DISALLOW_MOVE(TriggerConnectorMgr);
};
}  // namespace IntellVoiceTrigger
}  // namespace OHOS
#endif