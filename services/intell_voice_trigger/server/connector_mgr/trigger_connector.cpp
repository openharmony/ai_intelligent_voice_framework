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
#include <thread>

#include "trigger_connector_internal_impl.h"
#include "intell_voice_log.h"
#include "v1_1/iintell_voice_trigger_manager.h"
#include "scope_guard.h"
#include "trigger_callback_impl.h"
#include "memory_guard.h"
#include "iproxy_broker.h"
#include "intell_voice_death_recipient.h"

#define LOG_TAG "TriggerConnector"

using namespace std;
using namespace OHOS::HDI::ServiceManager::V1_0;
using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::HDI::IntelligentVoice::Trigger::V1_0;
using namespace OHOS::HDI::IntelligentVoice::Trigger::V1_1;

namespace OHOS {
namespace IntellVoiceTrigger {
static constexpr int32_t MAX_GET_HDI_SERVICE_TIMEOUT = 3;
static constexpr uint32_t MAX_RECOGNITION_EVENT_NUM = 100;
static const std::string THREAD_NAME = "TriggerThread_";

TriggerConnector::TriggerConnector(const IntellVoiceTriggerAdapterDsecriptor &desc)
{
    desc_.adapterName = desc.adapterName;
    LoadHdiAdapter();
}

TriggerConnector::~TriggerConnector()
{
    TriggerHostManager::UnloadTriggerAdapter(desc_);
}

bool TriggerConnector::LoadHdiAdapter()
{
    if (!TriggerHostManager::Init()) {
        INTELL_VOICE_LOG_WARN("failed to init trigger host manager");
        return false;
    }

    TriggerHostManager::RegisterTriggerHDIDeathRecipient();
    if (!TriggerHostManager::LoadTriggerAdapter(desc_)) {
        INTELL_VOICE_LOG_ERROR("failed to load trigger adapter");
        return false;
    }

    return true;
}

std::shared_ptr<IIntellVoiceTriggerConnectorModule> TriggerConnector::GetModule(
    std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (callback == nullptr) {
        INTELL_VOICE_LOG_ERROR("callback is nullptr");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    do {
        if (TriggerHostManager::GetAdapter() != nullptr) {
            INTELL_VOICE_LOG_INFO("already load hdi adapter");
            break;
        }
        {
            std::unique_lock<mutex> serviceStateLock(serviceStateMutex_);
            if (serviceState_ != SERVIE_STATUS_START) {
                if (!cv_.wait_for(serviceStateLock, chrono::seconds(MAX_GET_HDI_SERVICE_TIMEOUT),
                    [&] { return serviceState_ == SERVIE_STATUS_START; })) {
                    INTELL_VOICE_LOG_ERROR("time out");
                    return nullptr;
                }
            }
        }
        INTELL_VOICE_LOG_INFO("start to load hdi adapter");
        if (!LoadHdiAdapter()) {
            return nullptr;
        }
    } while (0);

    std::shared_ptr<TriggerSession> session = std::make_shared<TriggerSession>(this, callback, activeSessions_.size());
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

void TriggerConnector::OnReceive(const ServiceStatus &serviceStatus)
{
    if (serviceStatus.serviceName != INTELL_VOICE_TRIGGER_SERVICE) {
        return;
    }

    INTELL_VOICE_LOG_INFO("status:%{public}d", serviceStatus.status);
    {
        std::unique_lock<mutex> serviceSatetLock(serviceStateMutex_);
        if (serviceStatus.status == serviceState_) {
            return;
        }
        serviceState_ = serviceStatus.status;
    }
    cv_.notify_all();
}

TriggerConnector::TriggerSession::TriggerSession(TriggerConnector *connector,
    std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback, uint32_t threadId)
    : TaskExecutor(THREAD_NAME + std::to_string(threadId), MAX_RECOGNITION_EVENT_NUM),
    connector_(connector), callback_(callback)
{
    TaskExecutor::StartThread();
}

TriggerConnector::TriggerSession::~TriggerSession()
{
    loadedModels_.clear();
    TaskExecutor::StopThread();
}

int32_t TriggerConnector::TriggerSession::LoadModel(std::shared_ptr<GenericTriggerModel> model, int32_t &modelHandle)
{
    std::lock_guard<std::mutex> lock(connector_->mutex_);
    if (connector_->TriggerHostManager::GetAdapter() == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }
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
    std::lock_guard<std::mutex> lock(connector_->mutex_);
    if (connector_->TriggerHostManager::GetAdapter() == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }
    auto it = loadedModels_.find(modelHandle);
    if ((it == loadedModels_.end()) || (it->second == nullptr)) {
        INTELL_VOICE_LOG_ERROR("failed to find model");
        return -1;
    }

    int32_t ret = it->second->Unload();
    loadedModels_.erase(it);
    return ret;
}

int32_t TriggerConnector::TriggerSession::Start(int32_t modelHandle)
{
    std::lock_guard<std::mutex> lock(connector_->mutex_);
    if (connector_->TriggerHostManager::GetAdapter() == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }
    auto it = loadedModels_.find(modelHandle);
    if ((it == loadedModels_.end()) || (it->second == nullptr)) {
        INTELL_VOICE_LOG_ERROR("failed to find model");
        return -1;
    }
    return it->second->Start();
}

int32_t TriggerConnector::TriggerSession::Stop(int32_t modelHandle)
{
    std::lock_guard<std::mutex> lock(connector_->mutex_);
    if (connector_->TriggerHostManager::GetAdapter() == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }
    auto it = loadedModels_.find(modelHandle);
    if ((it == loadedModels_.end()) || (it->second == nullptr)) {
        INTELL_VOICE_LOG_ERROR("failed to find model");
        return -1;
    }
    return it->second->Stop();
}

int32_t TriggerConnector::TriggerSession::SetParams(const std::string& key, const std::string& value)
{
    std::lock_guard<std::mutex> lock(connector_->mutex_);
    if (connector_->TriggerHostManager::GetAdapterV1_1() == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }

    int32_t ret = connector_->TriggerHostManager::GetAdapterV1_1()->SetParams(key, value);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("failed to set params");
    }

    return ret;
}

int32_t TriggerConnector::TriggerSession::GetParams(const std::string& key, std::string &value)
{
    std::lock_guard<std::mutex> lock(connector_->mutex_);
    if (connector_->TriggerHostManager::GetAdapterV1_1() == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }

    return connector_->TriggerHostManager::GetAdapterV1_1()->GetParams(key, value);
}

void TriggerConnector::TriggerSession::HandleRecognitionHdiEvent(
    std::shared_ptr<IntellVoiceRecognitionEvent> event, int32_t modelHandle)
{
    std::function<void()> func = std::bind(
        &TriggerConnector::TriggerSession::ProcessRecognitionHdiEvent, this, event, modelHandle);
    TaskExecutor::AddAsyncTask(func);
}

void TriggerConnector::TriggerSession::ProcessRecognitionHdiEvent(
    std::shared_ptr<IntellVoiceRecognitionEvent> event, int32_t modelHandle)
{
    if (event == nullptr) {
        INTELL_VOICE_LOG_ERROR("event is nullptr");
        return;
    }
    if (connector_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("connector is nullptr");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(connector_->mutex_);
        auto it = loadedModels_.find(modelHandle);
        if ((it == loadedModels_.end()) || (it->second == nullptr)) {
            INTELL_VOICE_LOG_ERROR("can not find model, handle:%{public}d",  modelHandle);
            return;
        }

        if (it->second->GetState() != Model::ModelState::ACTIVE) {
            INTELL_VOICE_LOG_ERROR("unexpected recognition event, handle:%{public}d",  modelHandle);
            return;
        }

        it->second->SetState(Model::ModelState::LOADED);
    }
    callback_->OnRecognition(modelHandle, *(event.get()));
}

std::shared_ptr<TriggerConnector::TriggerSession::Model> TriggerConnector::TriggerSession::Model::Create(
    TriggerSession *session)
{
    return std::shared_ptr<Model>(new (std::nothrow) Model(session));
}

int32_t TriggerConnector::TriggerSession::Model::Load(std::shared_ptr<GenericTriggerModel> model, int32_t &modelHandle)
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

    ON_SCOPE_EXIT
    {
        INTELL_VOICE_LOG_INFO("close ashmem");
        triggerModel.data->UnmapAshmem();
        triggerModel.data->CloseAshmem();
    };

    int32_t handle;
    int32_t ret = session_->GetTriggerConnector()->TriggerHostManager::GetAdapter()->LoadModel(triggerModel,
        callback_, 0, handle);
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

    int32_t ret = session_->GetTriggerConnector()->TriggerHostManager::GetAdapter()->Start(handle_);
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

    int32_t ret = session_->GetTriggerConnector()->TriggerHostManager::GetAdapter()->Stop(handle_);
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

    int32_t ret = session_->GetTriggerConnector()->TriggerHostManager::GetAdapter()->UnloadModel(handle_);
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

void TriggerConnector::TriggerSession::Model::OnRecognitionHdiEvent(
    const IntellVoiceRecognitionEvent &event, int32_t cookie)
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

sptr<Ashmem> TriggerConnector::TriggerSession::Model::CreateAshmemFromModelData(const std::vector<uint8_t> &modelData)
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