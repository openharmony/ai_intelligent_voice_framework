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
#include "trigger_helper.h"
#include "intell_voice_log.h"

#include "trigger_connector_mgr.h"

using namespace OHOS::HDI::IntelligentVoice::Trigger::V1_0;
using namespace std;
#define LOG_TAG "TriggerHelper"

namespace OHOS {
namespace IntellVoiceTrigger {
TriggerModelData::TriggerModelData(int32_t uuid)
{
    uuid_ = uuid;
}

TriggerModelData::~TriggerModelData()
{
}

void TriggerModelData::SetCallback(std::shared_ptr<IIntellVoiceTriggerRecognitionCallback> callback)
{
    if (callback == nullptr) {
        INTELL_VOICE_LOG_ERROR("callback is nullptr");
        return;
    }
    callback_ = callback;
}

std::shared_ptr<IIntellVoiceTriggerRecognitionCallback> TriggerModelData::GetCallback()
{
    return callback_;
}

void TriggerModelData::SetModel(std::shared_ptr<GenericTriggerModel> model)
{
    if (SameModel(model)) {
        INTELL_VOICE_LOG_INFO("same model not need to update");
        return;
    }
    model_ = model;
    model_->Print();
}

shared_ptr<GenericTriggerModel> TriggerModelData::GetModel()
{
    return model_;
}

bool TriggerModelData::SameModel(std::shared_ptr<GenericTriggerModel> model)
{
    if (model == nullptr || model_ == nullptr) {
        return false;
    }
    return model_->GetData() == model->GetData();
}

void TriggerModelData::SetState(ModelState state)
{
    state_ = state;
}

ModelState TriggerModelData::GetState() const
{
    return state_;
}

void TriggerModelData::SetModelHandle(int32_t handle)
{
    modelHandle_ = handle;
}

int32_t TriggerModelData::GetModelHandle() const
{
    return modelHandle_;
}

void TriggerModelData::Clear()
{
    callback_ = nullptr;
}

TriggerHelper::TriggerHelper()
{
    moduleDesc_ = TriggerConnectorMgr::GetInstance()->ListConnectorModuleDescriptors();
}

TriggerHelper::~TriggerHelper()
{
    modelDataMap_.clear();
}

std::shared_ptr<TriggerHelper> TriggerHelper::Create()
{
    return std::shared_ptr<TriggerHelper>(new (std::nothrow) TriggerHelper());
}

int32_t TriggerHelper::StartGenericRecognition(int32_t uuid, std::shared_ptr<GenericTriggerModel> model,
    shared_ptr<IIntellVoiceTriggerRecognitionCallback> callback)
{
    INTELL_VOICE_LOG_INFO("enter");
    lock_guard<std::mutex> lock(mutex_);

    auto modelData = GetTriggerModelData(uuid);
    if (modelData == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to get trigger model data");
        return -1;
    }

    GetModule();

    bool unload = !modelData->SameModel(model);
    int32_t ret = InitRecognition(modelData, unload);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("failed to initialize recognition");
        return -1;
    }

    modelData->SetModel(model);
    modelData->SetCallback(callback);

    LoadModel(modelData);
    return StartRecognition(modelData);
}

int32_t TriggerHelper::StopGenericRecognition(
    int32_t uuid, shared_ptr<IIntellVoiceTriggerRecognitionCallback> callback)
{
    INTELL_VOICE_LOG_INFO("enter");
    lock_guard<std::mutex> lock(mutex_);
    auto modelData = GetTriggerModelData(uuid);
    if (modelData == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to get trigger model data");
        return -1;
    }
    modelData->SetCallback(callback);
    return StopRecognition(modelData);
}

void TriggerHelper::UnloadGenericTriggerModel(int32_t uuid)
{
    INTELL_VOICE_LOG_INFO("enter");
    lock_guard<std::mutex> lock(mutex_);
    auto modelData = GetTriggerModelData(uuid);
    if (modelData == nullptr) {
        INTELL_VOICE_LOG_WARN("no trigger model data");
        return;
    }

    if (modelData->GetState() == MODEL_NOTLOADED) {
        INTELL_VOICE_LOG_INFO("model is not loaded");
        return;
    }
    StopRecognition(modelData);
    UnloadModel(modelData);
}

void TriggerHelper::GetModule()
{
    if (module_ != nullptr) {
        return;
    }
    if (moduleDesc_.size() == 0) {
        INTELL_VOICE_LOG_INFO("moduleDesc_ is empty");
        return;
    }
    module_ =
        TriggerConnectorMgr::GetInstance()->GetConnectorModule(moduleDesc_[0].adapterName, shared_from_this());
    if (module_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to get connector module");
    }
}

int32_t TriggerHelper::InitRecognition(std::shared_ptr<TriggerModelData> modelData, bool unload)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (modelData->GetState() == MODEL_NOTLOADED) {
        return 0;
    }
    int32_t ret = StopRecognition(modelData);
    if (unload) {
        ret = UnloadModel(modelData);
    }
    modelData->Clear();
    return ret;
}

int32_t TriggerHelper::StartRecognition(shared_ptr<TriggerModelData> modelData)
{
    if (modelData == nullptr) {
        INTELL_VOICE_LOG_ERROR("modelData is nullptr");
        return -1;
    }
    if (modelData->GetState() != MODEL_LOADED) {
        return 0;
    }
    INTELL_VOICE_LOG_INFO("enter");
    if (module_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("module_ is nullptr");
        return -1;
    }
    auto ret = module_->Start(modelData->GetModelHandle());
    modelData->SetState(MODEL_STARTED);
    return ret;
}

int32_t TriggerHelper::StopRecognition(shared_ptr<TriggerModelData> modelData)
{
    if (modelData == nullptr) {
        INTELL_VOICE_LOG_ERROR("modelData is nullptr");
        return -1;
    }
    if (modelData->GetState() != MODEL_STARTED) {
        return 0;
    }
    INTELL_VOICE_LOG_INFO("enter");
    if (module_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("module_ is nullptr");
        return -1;
    }
    auto ret = module_->Stop(modelData->GetModelHandle());
    modelData->SetState(MODEL_LOADED);
    return ret;
}

int32_t TriggerHelper::LoadModel(shared_ptr<TriggerModelData> modelData)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (modelData == nullptr) {
        INTELL_VOICE_LOG_ERROR("modelData is nullptr");
        return -1;
    }
    if (modelData->GetState() != MODEL_NOTLOADED) {
        INTELL_VOICE_LOG_WARN("model is already loaded");
        return 0;
    }

    if (module_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("module_ is nullptr");
        return -1;
    }

    int32_t handle;
    auto ret = module_->LoadModel(modelData->GetModel(), handle);
    modelData->SetModelHandle(handle);
    modelData->SetState(MODEL_LOADED);
    INTELL_VOICE_LOG_INFO("exit, handle: %{public}d", handle);
    return ret;
}

int32_t TriggerHelper::UnloadModel(shared_ptr<TriggerModelData> modelData)
{
    if (modelData == nullptr) {
        INTELL_VOICE_LOG_ERROR("modelData is nullptr");
        return -1;
    }
    if (modelData->GetState() != MODEL_LOADED) {
        return 0;
    }
    INTELL_VOICE_LOG_INFO("enter");
    if (module_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("module_ is nullptr");
        return -1;
    }
    auto ret = module_->UnloadModel(modelData->GetModelHandle());
    modelData->SetState(MODEL_NOTLOADED);
    return ret;
}

shared_ptr<TriggerModelData> TriggerHelper::GetTriggerModelData(int32_t uuid)
{
    INTELL_VOICE_LOG_INFO("enter, uuid is :%{public}d", uuid);
    auto it = modelDataMap_.find(uuid);
    if (it != modelDataMap_.end() && it->second != nullptr) {
        return it->second;
    }

    auto modelData = std::make_shared<TriggerModelData>(uuid);
    if (modelData == nullptr) {
        INTELL_VOICE_LOG_INFO("modelData is nullptr");
        return nullptr;
    }
    modelDataMap_.insert(std::make_pair(uuid, modelData));
    return modelData;
}

void TriggerHelper::OnRecognition(int32_t modelHandle, const IntellVoiceRecognitionEvent &event)
{
    INTELL_VOICE_LOG_INFO("enter, modelHandle:%{public}d", modelHandle);
    lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<IIntellVoiceTriggerRecognitionCallback> callback = nullptr;
    for (auto iter : modelDataMap_) {
        if (iter.second == nullptr) {
            INTELL_VOICE_LOG_ERROR("uuid: %{public}d, model data is nullptr", iter.first);
            continue;
        }

        if (iter.second->GetModelHandle() == modelHandle) {
            iter.second->SetState(MODEL_LOADED);
            callback = iter.second->GetCallback();
            break;
        }
    }

    if (callback == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger recognition callback is nullptr, modelHandle: %{public}d", modelHandle);
        return;
    }

    auto genericEvent = std::make_shared<GenericTriggerEvent>();
    if (genericEvent == nullptr) {
        INTELL_VOICE_LOG_ERROR("genericEvent is nullptr");
        return;
    }
    genericEvent->modelHandle_ = modelHandle;
    callback->OnGenericTriggerDetected(genericEvent);
}
}  // namespace IntellVoiceTrigger
}  // namespace OHOS