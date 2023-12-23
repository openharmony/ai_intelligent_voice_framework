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

#include "trigger_service.h"

#include <malloc.h>

#include "intell_voice_log.h"

#define LOG_TAG "TriggerService"
using namespace std;

namespace OHOS {
namespace IntellVoiceTrigger {
TriggerService::TriggerService()
{
    dbHelper_ = std::make_shared<TriggerDbHelper>();
    if (dbHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("dbHelper_ is nullptr");
    }

    triggerHelper_ = TriggerHelper::Create();
    if (triggerHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("triggerHelper_ is nullptr");
    }
}

TriggerService::~TriggerService()
{}

void TriggerService::UpdateGenericTriggerModel(std::shared_ptr<GenericTriggerModel> model)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (dbHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("dbHelper_ is nullptr");
        return;
    }

    if (model == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger model is null");
        return;
    }

    if (!dbHelper_->UpdateGenericTriggerModel(model)) {
        INTELL_VOICE_LOG_ERROR("failed to update generic model");
    }
}

void TriggerService::DeleteGenericTriggerModel(int32_t uuid)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (dbHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("dbHelper_ is nullptr");
        return;
    }
    dbHelper_->DeleteGenericTriggerModel(uuid);
}

std::shared_ptr<GenericTriggerModel> TriggerService::GetGenericTriggerModel(int32_t uuid)
{
    INTELL_VOICE_LOG_INFO("enter");

    if (dbHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("dbHelper_ is nullptr");
        return nullptr;
    }

    auto model = dbHelper_->GetGenericTriggerModel(uuid);
    if (model == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to get generic trigger model");
        return nullptr;
    }

    return model;
}

int32_t TriggerService::StartRecognition(
    int32_t uuid, std::shared_ptr<IIntellVoiceTriggerRecognitionCallback> callback)
{
    if (triggerHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger helper is nullptr");
        return -1;
    }

    auto model = GetGenericTriggerModel(uuid);
    if (model == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger model is nullptr, uuid:%{public}d", uuid);
        return -1;
    }

    return triggerHelper_->StartGenericRecognition(uuid, model, callback);
}

int32_t TriggerService::StopRecognition(int32_t uuid, std::shared_ptr<IIntellVoiceTriggerRecognitionCallback> callback)
{
    if (triggerHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger helper is nullptr");
        return -1;
    }
    return triggerHelper_->StopGenericRecognition(uuid, callback);
}

void TriggerService::UnloadTriggerModel(int32_t uuid)
{
    if (triggerHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger helper is nullptr");
        return;
    }

    triggerHelper_->UnloadGenericTriggerModel(uuid);
}

int32_t TriggerService::SetParameter(const std::string &key, const std::string &value)
{
    if (triggerHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger helper is nullptr");
        return -1;
    }

    return triggerHelper_->SetParameter(key, value);
}

std::string TriggerService::GetParameter(const std::string &key)
{
    if (triggerHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger helper is nullptr");
        return "";
    }

    return triggerHelper_->GetParameter(key);
}

void TriggerService::AttachTelephonyObserver()
{
    if (triggerHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger helper is nullptr");
        return;
    }
    triggerHelper_->AttachTelephonyObserver();
}

void TriggerService::DetachTelephonyObserver()
{
    if (triggerHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger helper is nullptr");
        return;
    }
    triggerHelper_->DetachTelephonyObserver();
}

void TriggerService::AttachAudioCaptureListener()
{
    if (triggerHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger helper is nullptr");
        return;
    }
    triggerHelper_->AttachAudioCaptureListener();
}

void TriggerService::DetachAudioCaptureListener()
{
    if (triggerHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger helper is nullptr");
        return;
    }
    triggerHelper_->DetachAudioCaptureListener();
}

void TriggerService::AttachAudioRendererEventListener()
{
    if (triggerHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger helper is nullptr");
        return;
    }
    triggerHelper_->AttachAudioRendererEventListener();
}

void TriggerService::DetachAudioRendererEventListener()
{
    if (triggerHelper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger helper is nullptr");
        return;
    }
    triggerHelper_->DetachAudioRendererEventListener();
}
}  // namespace IntellVoiceTrigger
}  // namespace OHOS
