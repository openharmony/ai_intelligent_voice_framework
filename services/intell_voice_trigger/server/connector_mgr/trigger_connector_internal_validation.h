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
#ifndef INTELL_VOICE_TRIGGER_CONNECTOR_INTERNAL_VALIDATION_H
#define INTELL_VOICE_TRIGGER_CONNECTOR_INTERNAL_VALIDATION_H

#include <mutex>
#include <string>
#include <set>
#include "v1_0/intell_voice_trigger_types.h"
#include "i_intell_voice_trigger_connector_internal.h"

namespace OHOS {
namespace IntellVoiceTrigger {
using OHOS::HDI::IntelligentVoice::Trigger::V1_0::IntellVoiceRecognitionEvent;

class TriggerConnectorInternalValidation : public IIntellVoiceTriggerConnectorInternal {
public:
    explicit TriggerConnectorInternalValidation(std::unique_ptr<IIntellVoiceTriggerConnectorInternal> delegate);
    std::vector<TriggerConnectorModuleDesc> ListModuleDescriptors() override;
    std::shared_ptr<IIntellVoiceTriggerConnectorModule> GetModule(const std::string &adapterName,
        std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback) override;

private:
    std::mutex mutex_;
    std::set<std::string> moduleDescs_;
    std::unique_ptr<IIntellVoiceTriggerConnectorInternal> delegate_ = nullptr;

private:
    class TriggerConnectorModuleValidation : public IIntellVoiceTriggerConnectorModule {
    public:
        explicit TriggerConnectorModuleValidation(std::shared_ptr<IIntellVoiceTriggerConnectorCallback> callback);
        std::shared_ptr<IIntellVoiceTriggerConnectorCallback> GetCallbackWrapper()
        {
            return callbackWrapper_;
        }
        void SetDelegate(std::shared_ptr<IIntellVoiceTriggerConnectorModule> delegate)
        {
            delegate_ = delegate;
        }
        int32_t LoadModel(std::shared_ptr<GenericTriggerModel> model, int32_t &modelHandle) override;
        int32_t UnloadModel(int32_t modelHandle) override;
        int32_t Start(int32_t modelHandle) override;
        int32_t Stop(int32_t modelHandle) override;
        int32_t SetParams(const std::string &key, const std::string &value) override;
        int32_t GetParams(const std::string& key, std::string &value) override;

    private:
        class TriggerConnectorCallbackValidation : public IIntellVoiceTriggerConnectorCallback {
        public:
            explicit TriggerConnectorCallbackValidation(std::shared_ptr<IIntellVoiceTriggerConnectorCallback> delegate)
                : delegate_(delegate) {}
            void OnRecognition(int32_t modelHandle, const IntellVoiceRecognitionEvent &event) override
            {
                delegate_->OnRecognition(modelHandle, event);
            }

        private:
            std::shared_ptr<IIntellVoiceTriggerConnectorCallback> delegate_;
        };

        class ValidationUtils {
        public:
            static bool ValidateGenericModel(std::shared_ptr<GenericTriggerModel> model);
        };
    private:
        std::shared_ptr<IIntellVoiceTriggerConnectorModule> delegate_ = nullptr;
        std::shared_ptr<TriggerConnectorCallbackValidation> callbackWrapper_ = nullptr;
    };
};
}
}
#endif
