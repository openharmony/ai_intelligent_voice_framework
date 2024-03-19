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
#ifndef INTELL_VOICE_TRIGGER_DB_HELPER_H
#define INTELL_VOICE_TRIGGER_DB_HELPER_H
#include <string>
#include <mutex>

#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "trigger_base_type.h"

namespace OHOS {
namespace IntellVoiceTrigger {
class TriggerDbHelper {
public:
    TriggerDbHelper();
    ~TriggerDbHelper();
    bool UpdateGenericTriggerModel(std::shared_ptr<GenericTriggerModel> model);
    std::shared_ptr<GenericTriggerModel> GetGenericTriggerModel(const int32_t modelUuid);
    void DeleteGenericTriggerModel(const int32_t modelUuid);

private:
    bool GetVendorUuid(std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> &set, int32_t &vendorUuid) const;
    bool GetBlob(std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> &set, std::vector<uint8_t> &data) const;
    bool GetModelVersion(std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> &set, int32_t &version) const;
    bool GetModelType(std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> &set, int32_t &type) const;

private:
    std::mutex mutex_;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store_ = nullptr;
};
}
}
#endif
