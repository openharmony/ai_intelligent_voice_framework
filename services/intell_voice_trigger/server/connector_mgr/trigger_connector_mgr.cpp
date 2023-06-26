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
#include "trigger_connector_mgr.h"
#include "intell_voice_log.h"
#include "trigger_connector_internal_validation.h"
#include "trigger_connector_internal_impl.h"

#define LOG_TAG "TriggerConnectorMgr"

namespace OHOS {
namespace IntellVoiceTrigger {
std::unique_ptr<TriggerConnectorMgr> TriggerConnectorMgr::g_connectorMgr =
    std::unique_ptr<TriggerConnectorMgr>(
        new (std::nothrow) TriggerConnectorMgr(std::make_unique<TriggerConnectorInternalValidation>(
            std::make_unique<TriggerConnectorInternalImpl>())));

TriggerConnectorMgr::TriggerConnectorMgr(std::unique_ptr<IIntellVoiceTriggerConnectorInternal> delegate)
    : delegate_(std::move(delegate))
{}

std::unique_ptr<TriggerConnectorMgr> &TriggerConnectorMgr::GetInstance()
{
    return g_connectorMgr;
}

std::vector<TriggerConnectorModuleDesc> TriggerConnectorMgr::ListConnectorModuleDescriptors()
{
    if (delegate_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("delegate_ is nullptr");
        return {};
    }

    return delegate_->ListModuleDescriptors();
}

std::shared_ptr<IIntellVoiceTriggerConnectorModule> TriggerConnectorMgr::GetConnectorModule(
    const std::string &adapterName, std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback)
{
    if (delegate_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("delegate_ is nullptr");
        return nullptr;
    }

    return delegate_->GetModule(adapterName, callback);
}
}  // namespace IntellVoiceTrigger
}  // namespace OHOS