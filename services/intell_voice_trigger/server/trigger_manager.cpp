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
#include "intell_voice_log.h"
#include "memory_guard.h"

#define LOG_TAG "TriggerManager"

namespace OHOS {
namespace IntellVoiceTrigger {
std::mutex TriggerManager::instanceMutex_;
std::shared_ptr<TriggerManager> TriggerManager::instance_ = nullptr;

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

std::shared_ptr<TriggerManager> TriggerManager::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceMutex_);
        if (instance_ == nullptr) {
            instance_ = std::shared_ptr<TriggerManager>(new (std::nothrow) TriggerManager());
        }
    }
    return instance_;
}

void TriggerManager::UpdateModel(std::shared_ptr<GenericTriggerModel> model)
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
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

std::shared_ptr<GenericTriggerModel> TriggerManager::GetModel(int32_t uuid)
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return nullptr;
    }
    return service_->GetGenericTriggerModel(uuid);
}

std::shared_ptr<TriggerDetector> TriggerManager::CreateTriggerDetector(
    int32_t uuid, std::shared_ptr<IIntellVoiceTriggerDetectorCallback> callback)
{
    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    std::shared_ptr<TriggerDetector> detector = std::make_shared<TriggerDetector>(uuid, service_, callback);
    if (detector == nullptr) {
        INTELL_VOICE_LOG_ERROR("detector is nullptr");
        return nullptr;
    }

    detectors_[uuid] = detector;
    return detector;
}

void TriggerManager::ReleaseTriggerDetector(int32_t uuid)
{
    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    auto it = detectors_.find(uuid);
    if (it == detectors_.end()) {
        return;
    }

    detectors_.erase(it);
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
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    return service_->AttachTelephonyObserver();
}

void TriggerManager::DetachTelephonyObserver()
{
    if (service_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("service_ is nullptr");
        return;
    }
    return service_->DetachTelephonyObserver();
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
}  // namespace IntellVoiceTrigger
}  // namespace OHOS
