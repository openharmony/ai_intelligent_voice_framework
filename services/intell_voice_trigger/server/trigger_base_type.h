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
#ifndef INTELL_VOICE_TRIGGER_MODEL_H
#define INTELL_VOICE_TRIGGER_MODEL_H

#include <cstdint>
#include <memory>
#include <vector>

namespace OHOS {
namespace IntellVoiceTrigger {
class TriggerModel {
public:
    enum TriggerModelType {
        GENERIC_TYPE = 1,
        UNKNOWN_TYPE = -1,
    };

    TriggerModel(TriggerModelType type, int32_t uuid, int32_t version);
    virtual ~TriggerModel();

    bool SetData(const uint8_t *data, uint32_t size);
    bool SetData(std::vector<uint8_t> data);
    void Print();

    int32_t GetUuid()
    {
        return uuid_;
    }

    int32_t GetType()
    {
        return type_;
    }

    int32_t GetVendorUuid()
    {
        return vendorUuid_;
    }

    int32_t GetVersion()
    {
        return version_;
    }

    std::vector<uint8_t> GetData()
    {
        return data_;
    }

protected:
    TriggerModelType type_ = UNKNOWN_TYPE;
    int32_t uuid_ = -1;
    int32_t vendorUuid_ = -1;
    int32_t version_ = -1;

private:
    std::vector<uint8_t> data_;
};

class GenericTriggerModel : public TriggerModel {
public:
    GenericTriggerModel(int32_t uuid, int32_t version)
        : TriggerModel(TriggerModel::TriggerModelType::GENERIC_TYPE, uuid, version)
    {}
    ~GenericTriggerModel() override
    {}
};
}  // namespace IntellVoiceTrigger
}  // namespace OHOS

#endif