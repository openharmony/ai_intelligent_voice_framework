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
#include "trigger_connector_internal_impl.h"
#include <vector>
#include <iservmgr_hdi.h>
#include "intell_voice_log.h"
#include "v1_0/iintell_voice_trigger_manager.h"
#include "memory_guard.h"

#define LOG_TAG "TriggerConnectorInternalImpl"

using namespace OHOS::HDI::IntelligentVoice::Trigger::V1_0;

namespace OHOS {
namespace IntellVoiceTrigger {
TriggerConnectorInternalImpl::TriggerConnectorInternalImpl() : servmgr_(IServiceManager::Get())
{
    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    IntellVoiceTriggerAdapterDsecriptor descriptor;
    descriptor.adapterName = "primary";

    if (servmgr_ != nullptr) {
        auto connector = sptr<TriggerConnector>(new (std::nothrow) TriggerConnector(descriptor));
        if (connector != nullptr) {
            servmgr_->RegisterServiceStatusListener(connector, DEVICE_CLASS_DEFAULT);
            connectors_[descriptor.adapterName] = connector;
        } else {
            INTELL_VOICE_LOG_ERROR("failed to malloc connector");
        }
    } else {
        INTELL_VOICE_LOG_ERROR("failed to get hdf service manager");
    }
}

TriggerConnectorInternalImpl::~TriggerConnectorInternalImpl()
{
    INTELL_VOICE_LOG_DEBUG("TriggerConnectorInternalImpl destructor");
    if (servmgr_ != nullptr) {
        for (auto it = connectors_.begin(); it != connectors_.end(); ++it) {
            servmgr_->UnregisterServiceStatusListener(it->second);
        }
    }
}

std::vector<TriggerConnectorModuleDesc> TriggerConnectorInternalImpl::ListModuleDescriptors()
{
    std::vector<TriggerConnectorModuleDesc> ret;
    for (auto it = connectors_.begin(); it != connectors_.end(); ++it) {
        TriggerConnectorModuleDesc item;
        item.adapterName = it->first;
        item.properties = it->second->GetProperties();
        ret.emplace_back(item);
    }
    return ret;
}

std::shared_ptr<IIntellVoiceTriggerConnectorModule> TriggerConnectorInternalImpl::GetModule(
    const std::string &adapterName, std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback)
{
    auto it = connectors_.find(adapterName);
    if ((it == connectors_.end()) || (it->second == nullptr)) {
        INTELL_VOICE_LOG_ERROR("failed to find connector");
        return nullptr;
    }

    return it->second->GetModule(callback);
}
}
}
