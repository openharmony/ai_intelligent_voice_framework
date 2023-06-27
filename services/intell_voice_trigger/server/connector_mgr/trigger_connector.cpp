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
#ifdef TRIGGER_MANAGER_TEST
#include <thread>
#endif

#include "trigger_connector_internal_impl.h"
#include "intell_voice_log.h"
#include "v1_0/iintell_voice_trigger_manager.h"
#include "scope_guard.h"
#include "trigger_callback_impl.h"
#include "memory_guard.h"

#define LOG_TAG "TriggerConnector"

using namespace std;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::HDI::IntelligentVoice::Trigger::V1_0;

namespace OHOS {
namespace IntellVoiceTrigger {
TriggerConnector::TriggerConnector(const IntellVoiceTriggerAdapterDsecriptor &desc)
{
    desc_.adapterName = desc.adapterName;
    auto mgr = IIntellVoiceTriggerManager::Get();
    if (mgr != nullptr) {
        mgr->LoadAdapter(desc_, adapter_);
        if (adapter_ == nullptr) {
            INTELL_VOICE_LOG_ERROR("failed to load adapter, adapterName is %{public}s", desc_.adapterName.c_str());
        }
    } else {
        INTELL_VOICE_LOG_INFO("can not get intell voice trigger manager");
    }
}

TriggerConnector::~TriggerConnector()
{
    auto mgr = IIntellVoiceTriggerManager::Get();
    if (mgr != nullptr) {
        mgr->UnloadAdapter(desc_);
    }
    adapter_ = nullptr;
}

std::shared_ptr<IIntellVoiceTriggerConnectorModule> TriggerConnector::GetModule(
    std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback)
{
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return nullptr;
    }

    if (callback == nullptr) {
        INTELL_VOICE_LOG_ERROR("callback is nullptr");
        return nullptr;
    }

    std::shared_ptr<TriggerSession> session = std::make_shared<TriggerSession>(callback, adapter_);
    if (session == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to malloc session");
        return nullptr;
    }
    activeSessions_.insert(session);
    return session;
}

IntellVoiceTriggerProperties TriggerConnector::GetProperties()
{
    IntellVoiceTriggerProperties properties;
    return properties;
}

void TriggerConnector::OnReceive(const ServiceStatus &status)
{
    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    INTELL_VOICE_LOG_INFO("enter");
    if (status.serviceName != INTELL_VOICE_TRIGGER_SERVICE) {
        return;
    }
    if (adapter_ != nullptr) {
        return;
    }
    auto mgr = IIntellVoiceTriggerManager::Get();
    if (mgr != nullptr) {
        mgr->LoadAdapter(desc_, adapter_);
        if (adapter_ == nullptr) {
            INTELL_VOICE_LOG_ERROR("failed to load adapter, adapterName is %{public}s", desc_.adapterName.c_str());
        }
    } else {
        INTELL_VOICE_LOG_ERROR("failed to get trigger manager");
    }
}

TriggerConnector::TriggerSession::TriggerSession(
    std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback, const sptr<IIntellVoiceTriggerAdapter> &adapter)
    : callback_(callback), adapter_(adapter)
{
    BaseThread::Start();
}

TriggerConnector::TriggerSession::~TriggerSession()
{
    Message msg(MSG_TYPE_QUIT);
    SendMsg(msg);
    Join();
}

int32_t TriggerConnector::TriggerSession::LoadModel(
    std::shared_ptr<GenericTriggerModel> model, int32_t &modelHandle)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<Model> loadedModle = Model::Create(this);
    if (loadedModle == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to malloc intell voice model");
        return -1;
    }

    int32_t result = loadedModle->Load(model, modelHandle);
    if (result != 0) {
        INTELL_VOICE_LOG_ERROR("failed to load generic trigger model");
        return result;
    }
    loadedModels_.insert(std::make_pair(modelHandle, loadedModle));
    return result;
}

int32_t TriggerConnector::TriggerSession::UnloadModel(int32_t modelHandle)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = loadedModels_.find(modelHandle);
    if ((it == loadedModels_.end()) || (it->second == nullptr)) {
        INTELL_VOICE_LOG_ERROR("failed to find model");
        return -1;
    }
    return it->second->Unload();
}

int32_t TriggerConnector::TriggerSession::Start(int32_t modelHandle)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = loadedModels_.find(modelHandle);
    if ((it == loadedModels_.end()) || (it->second == nullptr)) {
        INTELL_VOICE_LOG_ERROR("failed to find model");
        return -1;
    }
    return it->second->Start();
}

int32_t TriggerConnector::TriggerSession::Stop(int32_t modelHandle)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = loadedModels_.find(modelHandle);
    if ((it == loadedModels_.end()) || (it->second == nullptr)) {
        INTELL_VOICE_LOG_ERROR("failed to find model");
        return -1;
    }
    return it->second->Stop();
}

void TriggerConnector::TriggerSession::HandleRecognitionHdiEvent(std::shared_ptr<IntellVoiceRecognitionEvent> event,
    int32_t modelHandle)
{
    Message msg(MSG_TYPE_RECOGNITION_HDI_EVENT);

    msg.obj2 = static_pointer_cast<void>(event);
    msg.arg1 = modelHandle;
    SendMsg(msg);
}

void TriggerConnector::TriggerSession::ProcessRecognitionHdiEvent(const Message &message)
{
    int32_t modelHandle = message.arg1;
    std::shared_ptr<IntellVoiceRecognitionEvent> event =
        static_pointer_cast<IntellVoiceRecognitionEvent>(message.obj2);
    if (event == nullptr) {
        INTELL_VOICE_LOG_ERROR("event is nullptr");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = loadedModels_.find(modelHandle);
        if ((it != loadedModels_.end()) && (it->second != nullptr)) {
            INTELL_VOICE_LOG_INFO("receive recognition event");
        }
    }
    callback_->OnRecognition(modelHandle, *(event.get()));
}

bool TriggerConnector::TriggerSession::HandleMsg(Message &message)
{
    bool quit = false;

    switch (message.mWhat) {
        case MSG_TYPE_RECOGNITION_HDI_EVENT:
            ProcessRecognitionHdiEvent(message);
            break;
        default:
            INTELL_VOICE_LOG_WARN("invalid msg id: %{public}d", message.mWhat);
            break;
    }

    if (message.mWhat == MSG_TYPE_QUIT) {
        quit = true;
    }

    return quit;
}

std::shared_ptr<TriggerConnector::TriggerSession::Model> TriggerConnector::TriggerSession::Model::Create(
    TriggerSession *session)
{
    return std::shared_ptr<Model>(new (std::nothrow) Model(session));
}

int32_t TriggerConnector::TriggerSession::Model::Load(
    std::shared_ptr<GenericTriggerModel> model, int32_t &modelHandle)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (GetState() != IDLE) {
        INTELL_VOICE_LOG_ERROR("model has already loaded");
        return -1;
    }

    callback_ = sptr<IIntellVoiceTriggerCallback>(new (std::nothrow) TriggerCallbackImpl(shared_from_this()));
    if (callback_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("callback_ is nullptr");
        return -1;
    }

    IntellVoiceTriggerModel triggerModel;
    triggerModel.data = CreateAshmemFromModelData(model->GetData());
    if (triggerModel.data == nullptr) {
        INTELL_VOICE_LOG_ERROR("data is nullptr");
        return -1;
    }
    triggerModel.type = static_cast<IntellVoiceTriggerModelType>(model->GetType());
    triggerModel.uid = static_cast<uint32_t>(model->GetUuid());

    ON_SCOPE_EXIT {
        INTELL_VOICE_LOG_INFO("close ashmem");
        triggerModel.data->UnmapAshmem();
        triggerModel.data->CloseAshmem();
    };

    int32_t handle;
    int32_t ret = session_->GetAdapter()->LoadModel(triggerModel, callback_, 0, handle);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("failed to load model");
        return ret;
    }
    (void)model;
    handle_ = handle;
    modelHandle = handle_;
    SetState(LOADED);
    return ret;
}

int32_t TriggerConnector::TriggerSession::Model::Start()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (GetState() != LOADED) {
        INTELL_VOICE_LOG_ERROR("model has not loaded");
        return -1;
    }

    int32_t ret = session_->GetAdapter()->Start(handle_);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("failed to load model");
        return ret;
    }

#ifdef TRIGGER_MANAGER_TEST
    std::thread(&TriggerConnector::TriggerSession::Model::TriggerManagerCallbackTest, this).detach();
#endif

    SetState(ACTIVE);
    return ret;
}

int32_t TriggerConnector::TriggerSession::Model::Stop()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (GetState() != ACTIVE) {
        INTELL_VOICE_LOG_ERROR("model has not activated");
        return -1;
    }

    int32_t ret = session_->GetAdapter()->Stop(handle_);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("failed to load model");
        return ret;
    }

    SetState(LOADED);
    return ret;
}

int32_t TriggerConnector::TriggerSession::Model::Unload()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (GetState() == IDLE) {
        INTELL_VOICE_LOG_ERROR("model has not loaded");
        return -1;
    }

    int32_t ret = session_->GetAdapter()->UnloadModel(handle_);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("failed to load model");
        return ret;
    }

    SetState(IDLE);
    return ret;
}

#ifdef TRIGGER_MANAGER_TEST
void TriggerConnector::TriggerSession::Model::TriggerManagerCallbackTest()
{
    INTELL_VOICE_LOG_ERROR("enter");
    IntellVoiceRecognitionEvent recognitionEvent;
    recognitionEvent.status = static_cast<RecognitionStatus>(0);
    recognitionEvent.type = static_cast<IntellVoiceTriggerModelType>(1);
    recognitionEvent.modelHandle = handle_;
    TriggerConnector::TriggerSession::Model::OnRecognitionHdiEvent(recognitionEvent, 0);
}
#endif

void TriggerConnector::TriggerSession::Model::OnRecognitionHdiEvent(const IntellVoiceRecognitionEvent& event,
    int32_t cookie)
{
    (void)cookie;
    std::shared_ptr<IntellVoiceRecognitionEvent> recognitionEvent = std::make_shared<IntellVoiceRecognitionEvent>();
    if (recognitionEvent == nullptr) {
        INTELL_VOICE_LOG_ERROR("recognitionEvent is nullptr");
        return;
    }

    recognitionEvent->status = event.status;
    recognitionEvent->type = event.type;
    recognitionEvent->modelHandle = event.modelHandle;

    INTELL_VOICE_LOG_INFO("handle: %{public}d", handle_);
    session_->HandleRecognitionHdiEvent(recognitionEvent, handle_);
}

sptr<Ashmem> TriggerConnector::TriggerSession::Model::CreateAshmemFromModelData(
    const std::vector<uint8_t> &modelData)
{
    if (modelData.size() == 0) {
        INTELL_VOICE_LOG_ERROR("data is empty");
        return nullptr;
    }

    sptr<Ashmem> buffer = OHOS::Ashmem::CreateAshmem("ModelData", modelData.size());
    if (buffer == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to create ashmem");
        return nullptr;
    }

    if (!buffer->MapReadAndWriteAshmem()) {
        INTELL_VOICE_LOG_ERROR("failed to map ashmem");
        goto ERR_EXIT;
    }

    if (!buffer->WriteToAshmem(modelData.data(), modelData.size(), 0)) {
        INTELL_VOICE_LOG_ERROR("failed to write ashmem");
        goto ERR_EXIT;
    }

    INTELL_VOICE_LOG_INFO("model data size:%{public}zu", modelData.size());
    return buffer;

ERR_EXIT:
    buffer->UnmapAshmem();
    buffer->CloseAshmem();
    return nullptr;
}
}  // namespace IntellVoiceTrigger
}  // namespace OHOS