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
#ifndef INTELL_VOICE_TRIGGER_CONNECTOR_INTERNAL_IMPL_H
#define INTELL_VOICE_TRIGGER_CONNECTOR_INTERNAL_IMPL_H

#include <map>
#include <vector>
#include "i_intell_voice_trigger_connector_internal.h"
#include "trigger_connector.h"

using OHOS::HDI::ServiceManager::V1_0::IServiceManager;

namespace OHOS {
namespace IntellVoiceTrigger {
class TriggerConnectorInternalImpl : public IIntellVoiceTriggerConnectorInternal {
public:
    TriggerConnectorInternalImpl();
    ~TriggerConnectorInternalImpl() override;
    std::vector<TriggerConnectorModuleDesc> ListModuleDescriptors() override;
    std::shared_ptr<IIntellVoiceTriggerConnectorModule> GetModule(const std::string &adapterName,
        std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback) override;

private:
    std::map<std::string, sptr<TriggerConnector>> connectors_;
    sptr<IServiceManager> servmgr_ = nullptr;
};
}
}
#endif