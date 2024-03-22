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
#include "trigger_connector_internal_validation.h"
#include "intell_voice_log.h"

#define LOG_TAG "TriggerConnectorValidation"

namespace OHOS {
namespace IntellVoiceTrigger {
TriggerConnectorInternalValidation::TriggerConnectorInternalValidation(
    std::unique_ptr<IIntellVoiceTriggerConnectorInternal> delegate) : delegate_(std::move(delegate))
{
}

std::vector<TriggerConnectorModuleDesc> TriggerConnectorInternalValidation::ListModuleDescriptors()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<TriggerConnectorModuleDesc> ret = delegate_->ListModuleDescriptors();
    if (ret.empty()) {
        INTELL_VOICE_LOG_ERROR("no trigger connector module desc");
        return ret;
    }

    if (moduleDescs_.empty()) {
        for (auto it : ret) {
            moduleDescs_.insert(it.adapterName);
        }
        return ret;
    }

    if (ret.size() != moduleDescs_.size()) {
        INTELL_VOICE_LOG_ERROR("size different, ret size:%zu, module descs size:%zu", ret.size(),
            moduleDescs_.size());
        return {};
    }

    for (auto it : ret) {
        if (moduleDescs_.count(it.adapterName) == 0) {
            INTELL_VOICE_LOG_ERROR("adapter name:%{public}s does not exist", it.adapterName.c_str());
            return {};
        }
    }

    return ret;
}

std::shared_ptr<IIntellVoiceTriggerConnectorModule> TriggerConnectorInternalValidation::GetModule(
    const std::string &adapterName, std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (callback == nullptr) {
        INTELL_VOICE_LOG_ERROR("callback is nullptr");
        return nullptr;
    }
    if (moduleDescs_.count(adapterName) == 0) {
        INTELL_VOICE_LOG_ERROR("adapter name:%{public}s does not exist", adapterName.c_str());
        return nullptr;
    }
    std::shared_ptr<TriggerConnectorModuleValidation> moduleValidation =
        std::make_shared<TriggerConnectorModuleValidation>(callback);
    if (moduleValidation == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to malloc connector module validation");
        return nullptr;
    }

    auto delegate = delegate_->GetModule(adapterName, moduleValidation->GetCallbackWrapper());
    if (delegate == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to get delegate");
        return nullptr;
    }
    moduleValidation->SetDelegate(delegate);
    return moduleValidation;
}

bool TriggerConnectorInternalValidation::TriggerConnectorModuleValidation::ValidationUtils::ValidateGenericModel(
    std::shared_ptr<GenericTriggerModel> model)
{
    if (model == nullptr) {
        INTELL_VOICE_LOG_ERROR("generic model is nullptr");
        return false;
    }

    if ((model->GetType() != TriggerModel::TriggerModelType::VOICE_WAKEUP_TYPE) &&
        (model->GetType() != TriggerModel::TriggerModelType::PROXIMAL_WAKEUP_TYPE)) {
        INTELL_VOICE_LOG_ERROR("generic model type:%{public}d is invalid", model->GetType());
        return false;
    }

    if (model->GetData().size() == 0) {
        INTELL_VOICE_LOG_ERROR("generic model data size is zero");
        return false;
    }

    return true;
}

TriggerConnectorInternalValidation::TriggerConnectorModuleValidation::TriggerConnectorModuleValidation(
    std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback)
{
    callbackWrapper_ = std::make_shared<TriggerConnectorCallbackValidation>(callback);
    if (callbackWrapper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to malloc callback wrapper");
    }
}

int32_t TriggerConnectorInternalValidation::TriggerConnectorModuleValidation::LoadModel(
    std::shared_ptr<GenericTriggerModel> model, int32_t &modelHandle)
{
    if (!ValidationUtils::ValidateGenericModel(model)) {
        INTELL_VOICE_LOG_ERROR();
        return -1;
    }
    return delegate_->LoadModel(model, modelHandle);
}

int32_t TriggerConnectorInternalValidation::TriggerConnectorModuleValidation::UnloadModel(
    int32_t modelHandle)
{
    return delegate_->UnloadModel(modelHandle);
}

int32_t TriggerConnectorInternalValidation::TriggerConnectorModuleValidation::Start(int32_t modelHandle)
{
    return delegate_->Start(modelHandle);
}

int32_t TriggerConnectorInternalValidation::TriggerConnectorModuleValidation::Stop(int32_t modelHandle)
{
    return delegate_->Stop(modelHandle);
}

int32_t TriggerConnectorInternalValidation::TriggerConnectorModuleValidation::SetParams(const std::string &key,
    const std::string &value)
{
    return delegate_->SetParams(key, value);
}

int32_t TriggerConnectorInternalValidation::TriggerConnectorModuleValidation::GetParams(const std::string &key,
    std::string &value)
{
    return delegate_->GetParams(key, value);
}
}
}
