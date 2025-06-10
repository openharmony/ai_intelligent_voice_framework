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
#include "trigger_manager.h"
#include "trigger_service.h"
#include "trigger_detector.h"
#include "intell_voice_log.h"
#include "memory_guard.h"
#include "trigger_detector_callback.h"

#define LOG_TAG "TriggerManager"

namespace OHOS {
namespace IntellVoiceTrigger {
TriggerManager::TriggerManager()
{
    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    service_ = std::make_shared<TriggerService>();
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
    }
}

TriggerManager::~TriggerManager()
{
    service_ = nullptr;
}

void TriggerManager::UpdateModel(std::vector<uint8_t> buffer, int32_t uuid, TriggerModelType type)
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    std::shared_ptr<GenericTriggerModel> model = std::make_shared<GenericTriggerModel>(uuid,
        TriggerModel::TriggerModelVersion::MODLE_VERSION_2, type);
    if (model == nullptr) {
        INTELL_VOICE_LOG_ERROR("model is null");
        return;
    }
    model->SetData(buffer);
    service_->UpdateGenericTriggerModel(model);
}

void TriggerManager::DeleteModel(int32_t uuid)
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    service_->DeleteGenericTriggerModel(uuid);
}

bool TriggerManager::IsModelExist(int32_t uuid)
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return false;
    }
    if (service_->GetGenericTriggerModel(uuid) == nullptr) {
        return false;
    }
    return true;
}

std::shared_ptr<GenericTriggerModel> TriggerManager::GetModel(int32_t uuid)
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return nullptr;
    }
    return service_->GetGenericTriggerModel(uuid);
}

void TriggerManager::ReleaseTriggerDetector(int32_t uuid)
{
    std::lock_guard<std::mutex> lock(detectorMutex_);
    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    auto it = detectors_.find(uuid);
    if (it != detectors_.end()) {
        if (it->second != nullptr) {
            it->second->UnloadTriggerModel();
        }
        detectors_.erase(it);
    }
}

int32_t TriggerManager::SetParameter(const std::string &key, const std::string &value)
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return -1;
    }
    return service_->SetParameter(key, value);
}

std::string TriggerManager::GetParameter(const std::string &key)
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return "";
    }
    return service_->GetParameter(key);
}


void TriggerManager::AttachTelephonyObserver()
{
#ifdef SUPPORT_TELEPHONY_SERVICE
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    return service_->AttachTelephonyObserver();
#endif
}

void TriggerManager::DetachTelephonyObserver()
{
#ifdef SUPPORT_TELEPHONY_SERVICE

    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    return service_->DetachTelephonyObserver();
#endif
}

void TriggerManager::AttachAudioCaptureListener()
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    return service_->AttachAudioCaptureListener();
}

void TriggerManager::DetachAudioCaptureListener()
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    return service_->DetachAudioCaptureListener();
}

void TriggerManager::AttachAudioRendererEventListener()
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    return service_->AttachAudioRendererEventListener();
}

void TriggerManager::DetachAudioRendererEventListener()
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    return service_->DetachAudioRendererEventListener();
}

void TriggerManager::AttachAudioSceneEventListener()
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    return service_->AttachAudioSceneEventListener();
}

void TriggerManager::DetachAudioSceneEventListener()
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    return service_->DetachAudioSceneEventListener();
}

#ifdef POWER_MANAGER_ENABLE
void TriggerManager::AttachHibernateObserver()
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    return service_->AttachHibernateObserver();
}

void TriggerManager::DetachHibernateObserver()
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    return service_->DetachHibernateObserver();
}
#endif

void TriggerManager::AttachFoldStatusListener()
{
#ifdef SUPPORT_WINDOW_MANAGER
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    return service_->AttachFoldStatusListener();
#endif
}

void TriggerManager::DetachFoldStatusListener()
{
#ifdef SUPPORT_WINDOW_MANAGER
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    return service_->DetachFoldStatusListener();
#endif
}

int32_t TriggerManager::StartDetection(int32_t uuid)
{
    std::lock_guard<std::mutex> lock(detectorMutex_);
    if ((detectors_.count(uuid) == 0) || (detectors_[uuid] == nullptr)) {
        INTELL_VOICE_LOG_INFO("detector is not existed, uuid:%{public}d", uuid);
        return -1;
    }

    if (service_->GetParameter("audio_hal_status") == "true") {
        INTELL_VOICE_LOG_INFO("audio hal is ready");
        detectors_[uuid]->StartRecognition();
        return 1;
    }
    return 0;
}

void TriggerManager::StopDetection(int32_t uuid)
{
    std::lock_guard<std::mutex> lock(detectorMutex_);
    if ((detectors_.count(uuid) == 0) || (detectors_[uuid] == nullptr)) {
        INTELL_VOICE_LOG_INFO("detector is not existed, uuid:%{public}d", uuid);
        return;
    }
    detectors_[uuid]->StopRecognition();
}

void TriggerManager::CreateDetector(int32_t uuid, std::function<void()> onDetected)
{
    std::lock_guard<std::mutex> lock(detectorMutex_);
    if (detectors_.count(uuid) != 0 && detectors_[uuid] != nullptr) {
        INTELL_VOICE_LOG_INFO("detector is already existed, no need to create, uuid:%{public}d", uuid);
        return;
    }

    auto cb = std::make_shared<TriggerDetectorCallback>(onDetected);
    if (cb == nullptr) {
        INTELL_VOICE_LOG_ERROR("cb is nullptr");
        return;
    }

    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    std::shared_ptr<TriggerDetector> detector = std::make_shared<TriggerDetector>(uuid, service_, cb);
    if (detector == nullptr) {
        INTELL_VOICE_LOG_ERROR("detector is nullptr");
        return;
    }

    detectors_[uuid] = detector;
}

void TriggerManager::OnServiceStart()
{
}

void TriggerManager::OnServiceStop()
{
    DetachTelephonyObserver();
    DetachAudioCaptureListener();
    DetachAudioRendererEventListener();
#ifdef POWER_MANAGER_ENABLE
    DetachHibernateObserver();
#endif
    DetachFoldStatusListener();
    DetachAudioSceneEventListener();
}

void TriggerManager::OnTelephonyStateRegistryServiceChange(bool isAdded)
{
#ifdef SUPPORT_TELEPHONY_SERVICE
    if (isAdded) {
        INTELL_VOICE_LOG_INFO("telephony state registry service is added");
        AttachTelephonyObserver();
    } else {
        INTELL_VOICE_LOG_INFO("telephony state registry service is removed");
    }
#endif
}

void TriggerManager::OnAudioDistributedServiceChange(bool isAdded)
{
    if (isAdded) {
        INTELL_VOICE_LOG_INFO("audio distributed service is added");
        AttachAudioCaptureListener();
    } else {
        INTELL_VOICE_LOG_INFO("audio distributed service is removed");
    }
}

void TriggerManager::OnAudioPolicyServiceChange(bool isAdded)
{
    if (isAdded) {
        INTELL_VOICE_LOG_INFO("audio policy service is added");
        AttachAudioRendererEventListener();
        AttachAudioSceneEventListener();
    } else {
        INTELL_VOICE_LOG_INFO("audio policy service is removed");
    }
}

#ifdef POWER_MANAGER_ENABLE
void TriggerManager::OnPowerManagerServiceChange(bool isAdded)
{
    if (isAdded) {
        INTELL_VOICE_LOG_INFO("power manager service is added");
        AttachHibernateObserver();
    } else {
        INTELL_VOICE_LOG_INFO("power manager service is removed");
    }
}
#endif

void TriggerManager::OnDisplayManagerServiceChange(bool isAdded)
{
#ifdef SUPPORT_WINDOW_MANAGER
    if (isAdded) {
        INTELL_VOICE_LOG_INFO("fold status service is added");
        AttachFoldStatusListener();
    } else {
        INTELL_VOICE_LOG_INFO("fold status service is removed");
    }
#endif
}
}  // namespace IntellVoiceTrigger
}  // namespace OHOS
