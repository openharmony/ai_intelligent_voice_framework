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
#include "trigger_base_type.h"
#include "securec.h"
#include "intell_voice_log.h"

#define LOG_TAG "TriggerBaseType"

namespace OHOS {
namespace IntellVoiceTrigger {
TriggerModel::TriggerModel(int32_t uuid, int32_t version, TriggerModelType type)
    : uuid_(uuid), version_(version), type_(type)
{}

TriggerModel::~TriggerModel()
{
}

bool TriggerModel::SetData(const uint8_t *data, uint32_t size)
{
    if (size == 0) {
        INTELL_VOICE_LOG_ERROR("size is invalid");
        return false;
    }
    data_.resize(size);

    if (memcpy_s(data_.data(), data_.size(), data, size) != 0) {
        INTELL_VOICE_LOG_ERROR("memcpy_s error");
        return false;
    }
    return true;
}

bool TriggerModel::SetData(std::vector<uint8_t> &data)
{
    if (data.size() == 0) {
        INTELL_VOICE_LOG_ERROR("size is invalid");
        return false;
    }
    data_.swap(data);
    return true;
}

void TriggerModel::Print()
{
    INTELL_VOICE_LOG_INFO("trigger model type:%{public}d", type_);
    INTELL_VOICE_LOG_INFO("trigger model uuid:%{public}d", uuid_);
    INTELL_VOICE_LOG_INFO("trigger model vendor uuid:%{public}d", vendorUuid_);
    INTELL_VOICE_LOG_INFO("trigger model version:%{public}d", version_);
    INTELL_VOICE_LOG_INFO("trigger model data size:%{public}zu", data_.size());
}
}  // namespace IntellVoiceTrigger
}  // namespace OHOS