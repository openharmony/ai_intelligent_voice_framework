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
#include "telephony_observer_client.h"
#include "state_registry_errors.h"
#include "telephony_types.h"
#include "call_manager_inner_type.h"

using namespace OHOS::HDI::IntelligentVoice::Trigger::V1_0;
using namespace OHOS::AudioStandard;
using namespace OHOS::Telephony;
using namespace std;
#define LOG_TAG "TriggerHelper"

namespace OHOS {
namespace IntellVoiceTrigger {

static constexpr int32_t SIM_SLOT_ID_1 = DEFAULT_SIM_SLOT_ID + 1;

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
    const auto &connectMgr = TriggerConnectorMgr::GetInstance();
    if (connectMgr != nullptr) {
        moduleDesc_ = connectMgr->ListConnectorModuleDescriptors();
    }
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
    modelDataMap_.erase(uuid);
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

bool TriggerHelper::IsConflictSceneActive()
{
    if (telephonyObserver0_ == nullptr ||
        telephonyObserver1_ == nullptr ||
        audioCapturerSourceChangeCallback_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("observe nullptr");
        return false;
    }

    INTELL_VOICE_LOG_INFO("callActive0_ %{public}d, callActive1_ %{public}d, audioCaptureActive_ %{public}d",
        telephonyObserver0_->callActive_,
        telephonyObserver1_->callActive_,
        audioCapturerSourceChangeCallback_->audioCaptureActive_);

    return (telephonyObserver0_->callActive_ ||
        telephonyObserver1_->callActive_ ||
        audioCapturerSourceChangeCallback_->audioCaptureActive_);
}

void TriggerHelper::OnUpdateAllRecognitionState()
{
    lock_guard<std::mutex> lock(mutex_);
    for (auto iter : modelDataMap_) {
        if (iter.second == nullptr) {
            INTELL_VOICE_LOG_ERROR("uuid: %{public}d, model data is nullptr", iter.first);
            continue;
        }
        if (IsConflictSceneActive()) {
            StopRecognition(iter.second);
        } else {
            StartRecognition(iter.second);
        }
    }
}

void TriggerHelper::AttachTelephonyObserver()
{
    INTELL_VOICE_LOG_INFO("enter");
    telephonyObserver0_ = std::make_unique<TelephonyStateObserver>(shared_from_this()).release();
    if (telephonyObserver0_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("telephonyObserver0_ is nullptr");
        return;
    }
    auto res = TelephonyObserverClient::GetInstance().AddStateObserver(telephonyObserver0_, DEFAULT_SIM_SLOT_ID,
        TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE, false);
    if (res != TELEPHONY_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("telephonyObserver0_ add failed");
    }

    telephonyObserver1_ = std::make_unique<TelephonyStateObserver>(shared_from_this()).release();
    if (telephonyObserver1_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("telephonyObserver1_ is nullptr");
        return;
    }
    res = TelephonyObserverClient::GetInstance().AddStateObserver(telephonyObserver1_, SIM_SLOT_ID_1,
        TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE, false);
    if (res != TELEPHONY_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("telephonyObserver1_ add failed");
    }
}

void TriggerHelper::DettachTelephonyObserver()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (telephonyObserver0_) {
        Telephony::TelephonyObserverClient::GetInstance().RemoveStateObserver(
            DEFAULT_SIM_SLOT_ID, Telephony::TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE);
        telephonyObserver0_ = nullptr;
    }

    if (telephonyObserver1_) {
        Telephony::TelephonyObserverClient::GetInstance().RemoveStateObserver(
            SIM_SLOT_ID_1, Telephony::TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE);
        telephonyObserver1_ = nullptr;
    }
}

void TriggerHelper::TelephonyStateObserver::OnCallStateUpdated(int32_t slotId, int32_t callState,
    const std::u16string &phoneNumber)
{
    if (helper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("helper is nullptr");
        return;
    }

    if (callState < static_cast<int32_t>(TelCallState::CALL_STATUS_UNKNOWN) ||
        callState > static_cast<int32_t>(TelCallState::CALL_STATUS_IDLE)) {
        INTELL_VOICE_LOG_ERROR("callstate err: %{public}d", callState);
        return;
    }

    bool curCallActive = (callState != static_cast<int32_t>(TelCallState::CALL_STATUS_DISCONNECTED) &&
        callState != static_cast<int32_t>(TelCallState::CALL_STATUS_IDLE) &&
        callState != static_cast<int32_t>(TelCallState::CALL_STATUS_UNKNOWN));

    INTELL_VOICE_LOG_INFO("state: %{public}d, slotId: %{public}d, callActive: %{public}d, curCallActive: %{public}d",
        callState, slotId, callActive_, curCallActive);
    if (callActive_ == curCallActive) {
        return;
    }

    callActive_ = curCallActive;
    helper_->OnUpdateAllRecognitionState();
}

void TriggerHelper::AttachAudioCaptureListener()
{
    INTELL_VOICE_LOG_INFO("enter");

    audioCapturerSourceChangeCallback_ = std::make_shared<AudioCapturerSourceChangeCallback>(shared_from_this());
    auto audioSystemManager = AudioSystemManager::GetInstance();
    if (audioSystemManager != nullptr) {
        audioSystemManager->SetAudioCapturerSourceCallback(audioCapturerSourceChangeCallback_);
    } else {
        INTELL_VOICE_LOG_ERROR("audioSystemManager is nullptr");
    }
}

void TriggerHelper::DettachAudioCaptureListener()
{
    INTELL_VOICE_LOG_INFO("enter");

    auto audioSystemManager = AudioSystemManager::GetInstance();
    if (audioSystemManager != nullptr) {
        audioSystemManager->SetAudioCapturerSourceCallback(nullptr);
    } else {
        INTELL_VOICE_LOG_ERROR("audioSystemManager is null");
    }
}

void TriggerHelper::AudioCapturerSourceChangeCallback::OnCapturerState(bool isActive)
{
    INTELL_VOICE_LOG_INFO("OnCapturerState active: %{public}d", isActive);

    if (helper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("helper is nullptr");
        return;
    }

    if (audioCaptureActive_ == isActive) {
        return;
    }

    audioCaptureActive_ = isActive;

    helper_->OnUpdateAllRecognitionState();
}
}  // namespace IntellVoiceTrigger
}  // namespace OHOS
