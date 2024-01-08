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
#ifndef I_INTELL_VOICE_TRIGGER_CONNECTOR_MODULE_H
#define I_INTELL_VOICE_TRIGGER_CONNECTOR_MODULE_H

#include <cstdint>
#include <memory>
#include "trigger_base_type.h"

namespace OHOS {
namespace IntellVoiceTrigger {
class IIntellVoiceTriggerConnectorModule {
public:
    IIntellVoiceTriggerConnectorModule() = default;
    virtual ~IIntellVoiceTriggerConnectorModule() = default;

    virtual int32_t LoadModel(std::shared_ptr<GenericTriggerModel> model, int32_t &modelHandle) = 0;
    virtual int32_t UnloadModel(int32_t modelHandle) = 0;
    virtual int32_t Start(int32_t modelHandle) = 0;
    virtual int32_t Stop(int32_t modelHandle) = 0;
    virtual int32_t SetParams(const std::string &key, const std::string &value) = 0;
    virtual int32_t GetParams(const std::string &key, std::string &value) = 0;
};
}
}
#endif