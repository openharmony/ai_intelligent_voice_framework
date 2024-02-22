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

#include "service_db_helper.h"
#include "intell_voice_log.h"

using namespace OHOS::DistributedKv;
#define LOG_TAG "ServiceDbHelper"

namespace OHOS {
namespace IntellVoiceEngine {
ServiceDbHelper::ServiceDbHelper(const std::string &inAppId, const std::string &inStoreId)
{
    AppId appId = { inAppId };
    StoreId storeId = { inStoreId };

    Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .securityLevel = SecurityLevel::S1,
        .area = Area::EL1,
        .kvStoreType = KvStoreType::SINGLE_VERSION,
        .baseDir = "/data/service/el1/public/database/" + appId.appId
    };

    INTELL_VOICE_LOG_INFO("inAppId:%{public}s, inStoreId:%{public}s, options.baseDir:%{public}s",
        inAppId.c_str(), appId.appId.c_str(), options.baseDir.c_str());

    DistributedKvDataManager manager;
    Status status = manager.GetSingleKvStore(options, appId, storeId, kvStore_);
    if (status != Status::SUCCESS) {
        INTELL_VOICE_LOG_INFO("GetSingleKvStore failed, status: %{public}d.", status);
    } else {
        INTELL_VOICE_LOG_INFO("GetSingleKvStore success");
    }
}

ServiceDbHelper::~ServiceDbHelper()
{
    kvStore_ = nullptr;
}

void ServiceDbHelper::SetValue(const std::string &key, const std::string &value)
{
    if (kvStore_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("kvStore_ is nullptr");
        return;
    }
    kvStore_->Put(key, value);
}

std::string ServiceDbHelper::GetValue(const std::string &key)
{
    if (kvStore_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("kvStore_ is nullptr");
        return "";
    }
    Value value;
    kvStore_->Get(key, value);
    return value.ToString();
}
}
}
