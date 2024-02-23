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
#ifndef INTELL_VOICE_TRIGGER_CONNECTOR_H
#define INTELL_VOICE_TRIGGER_CONNECTOR_H

#include <memory>
#include <set>
#include <map>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <iservmgr_hdi.h>
#include "i_intell_voice_trigger_adapter_listener.h"
#include "i_intell_voice_trigger_connector_module.h"
#include "i_intell_voice_trigger_connector_callback.h"
#include "msg_handle_thread.h"
#include "task_executor.h"
#include "trigger_host_manager.h"

namespace OHOS {
namespace IntellVoiceTrigger {
using OHOS::HDI::ServiceManager::V1_0::ServiceStatus;
using OHOS::HDI::ServiceManager::V1_0::ServStatListenerStub;

using OHOS::HDI::IntelligentVoice::Trigger::V1_0::IIntellVoiceTriggerCallback;
using OHOS::HDI::IntelligentVoice::Trigger::V1_0::IntellVoiceRecognitionEvent;
using OHOS::HDI::IntelligentVoice::Trigger::V1_0::IntellVoiceTriggerAdapterDsecriptor;
using OHOS::HDI::IntelligentVoice::Trigger::V1_0::IntellVoiceTriggerProperties;

const std::string INTELL_VOICE_TRIGGER_SERVICE = "intell_voice_trigger_manager_service";

class TriggerConnector : public ServStatListenerStub, private TriggerHostManager {
public:
    explicit TriggerConnector(const IntellVoiceTriggerAdapterDsecriptor &desc);
    ~TriggerConnector() override;
    std::shared_ptr<IIntellVoiceTriggerConnectorModule> GetModule(
        std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback);
    IntellVoiceTriggerProperties GetProperties();
    void OnReceive(const ServiceStatus &serviceStatus) override;
    void onServiceStart();

private:
    bool LoadHdiAdapter();

private:
    class TriggerSession : public IIntellVoiceTriggerConnectorModule, private OHOS::IntellVoiceUtils::TaskExecutor {
    public:
        TriggerSession(TriggerConnector *connector, std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback,
            uint32_t threadId);
        ~TriggerSession() override;
        TriggerConnector* GetTriggerConnector()
        {
            return connector_;
        }
        std::shared_ptr<IIntellVoiceTriggerConnectorCallback> GetCallback()
        {
            return callback_;
        }
        int32_t LoadModel(std::shared_ptr<GenericTriggerModel> model, int32_t &modelHandle) override;
        int32_t UnloadModel(int32_t modelHandle) override;
        int32_t Start(int32_t modelHandle) override;
        int32_t Stop(int32_t modelHandle) override;
        int32_t SetParams(const std::string &key, const std::string &value) override;
        int32_t GetParams(const std::string& key, std::string &value) override;

        void HandleRecognitionHdiEvent(std::shared_ptr<IntellVoiceRecognitionEvent> event, int32_t modelHandle);

    private:
        enum MsgType {
            MSG_TYPE_NONE,
            MSG_TYPE_RECOGNITION_HDI_EVENT,
            MSG_TYPE_QUIT,
        };

    private:
        class Model : public IIntellVoiceTriggerAdapterListener, public std::enable_shared_from_this<Model> {
        public:
            static std::shared_ptr<Model> Create(TriggerSession *session);
            int32_t Load(std::shared_ptr<GenericTriggerModel> model, int32_t &modelHandle);
            int32_t Unload();
            int32_t Start();
            int32_t Stop();
#ifdef TRIGGER_MANAGER_TEST
            void TriggerManagerCallbackTest();
#endif
        public:
            void OnRecognitionHdiEvent(const IntellVoiceRecognitionEvent &event, int32_t cookie) override;

            enum ModelState {
                IDLE,
                LOADED,
                ACTIVE,
            };
            ModelState GetState()
            {
                return state_.load();
            }

            void SetState(ModelState state)
            {
                state_.store(state);
            }

        private:
            explicit Model(TriggerSession *session) : session_(session)
            {}
            sptr<Ashmem> CreateAshmemFromModelData(const std::vector<uint8_t> &modelData);
            int32_t handle_ = 0;
            std::atomic<ModelState> state_ = IDLE;
            TriggerSession *session_ = nullptr;
            sptr<IIntellVoiceTriggerCallback> callback_ = nullptr;
        };  // Model

    private:
        void ProcessRecognitionHdiEvent(std::shared_ptr<IntellVoiceRecognitionEvent> event, int32_t modelHandle);

    private:
        TriggerConnector *connector_ = nullptr;
        std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback_ = nullptr;
        std::map<int32_t, std::shared_ptr<Model>> loadedModels_;
    };  // TriggerSession

private:
    std::mutex mutex_ {};
    std::mutex serviceStateMutex_ {};
    std::condition_variable cv_;
    IntellVoiceTriggerAdapterDsecriptor desc_;
    std::set<std::shared_ptr<TriggerSession>> activeSessions_;
    uint16_t serviceState_ = HDI::ServiceManager::V1_0::SERVIE_STATUS_MAX;
};
}  // namespace IntellVoiceTrigger
}  // namespace OHOS
#endif