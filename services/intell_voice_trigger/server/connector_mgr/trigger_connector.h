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
#include <iservmgr_hdi.h>
#include "v1_0/iintell_voice_trigger_adapter.h"
#include "i_intell_voice_trigger_adapter_listener.h"
#include "i_intell_voice_trigger_connector_module.h"
#include "i_intell_voice_trigger_connector_callback.h"
#include "msg_handle_thread.h"

namespace OHOS {
namespace IntellVoiceTrigger {
using OHOS::HDI::ServiceManager::V1_0::ServiceStatus;
using OHOS::HDI::ServiceManager::V1_0::ServStatListenerStub;

using OHOS::HDI::IntelligentVoice::Trigger::V1_0::IntellVoiceTriggerAdapterDsecriptor;
using OHOS::HDI::IntelligentVoice::Trigger::V1_0::IntellVoiceTriggerProperties;
using OHOS::HDI::IntelligentVoice::Trigger::V1_0::IIntellVoiceTriggerAdapter;
using OHOS::HDI::IntelligentVoice::Trigger::V1_0::IntellVoiceRecognitionEvent;
using OHOS::HDI::IntelligentVoice::Trigger::V1_0::IIntellVoiceTriggerCallback;

const std::string INTELL_VOICE_TRIGGER_SERVICE = "intell_voice_trigger_manager_service";

class TriggerConnector : public ServStatListenerStub  {
public:
    explicit TriggerConnector(const IntellVoiceTriggerAdapterDsecriptor &desc);
    ~TriggerConnector() override;
    std::shared_ptr<IIntellVoiceTriggerConnectorModule> GetModule(
        std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback);
    IntellVoiceTriggerProperties GetProperties();
    void OnReceive(const ServiceStatus &serviceStatus) override;

private:
    class TriggerSession : public IIntellVoiceTriggerConnectorModule, public OHOS::IntellVoiceUtils::MsgHandleThread {
    public:
        TriggerSession(std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback,
            const sptr<IIntellVoiceTriggerAdapter> &adapter);
        ~TriggerSession() override;
        const sptr<IIntellVoiceTriggerAdapter> &GetAdapter()
        {
            return adapter_;
        }
        std::shared_ptr<IIntellVoiceTriggerConnectorCallback> GetCallback()
        {
            return callback_;
        }
        int32_t LoadModel(std::shared_ptr<GenericTriggerModel> model, int32_t &modelHandle) override;
        int32_t UnloadModel(int32_t modelHandle) override;
        int32_t Start(int32_t modelHandle) override;
        int32_t Stop(int32_t modelHandle) override;

        void HandleRecognitionHdiEvent(std::shared_ptr<IntellVoiceRecognitionEvent> event,
            int32_t modelHandle);
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
            explicit Model(TriggerSession *session) : session_(session) {}
            sptr<Ashmem> CreateAshmemFromModelData(const std::vector<uint8_t> &modelData);
            int32_t handle_ = 0;
            std::atomic<ModelState> state_ = IDLE;
            TriggerSession *session_ = nullptr;
            sptr<IIntellVoiceTriggerCallback> callback_ = nullptr;
        }; // Model

    private:
        bool HandleMsg(OHOS::IntellVoiceUtils::Message &message) override;
        void ProcessRecognitionHdiEvent(const OHOS::IntellVoiceUtils::Message &message);
    private:
        std::mutex mutex_ {};
        std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback_ = nullptr;
        const OHOS::sptr<IIntellVoiceTriggerAdapter> &adapter_;
        std::map<int32_t, std::shared_ptr<Model>> loadedModels_;
    }; // TriggerSession

private:
    IntellVoiceTriggerAdapterDsecriptor desc_;
    sptr<IIntellVoiceTriggerAdapter> adapter_ = nullptr;
    std::set<std::shared_ptr<TriggerSession>> activeSessions_;
};
}  // namespace IntellVoiceTrigger
}  // namespace OHOS
#endif