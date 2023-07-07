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

#include "trigger_db_helper.h"

#include <string>

#include "intell_voice_log.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"

#define LOG_TAG "TriggerDbhelper"

using namespace OHOS::NativeRdb;

namespace OHOS {
namespace IntellVoiceTrigger {
class TriggerModelOpenCallback : public RdbOpenCallback {
public:
    int OnCreate(RdbStore &rdbStore) override;
    int OnUpgrade(RdbStore &rdbStore, int oldVersion, int newVersion) override;
};

int TriggerModelOpenCallback::OnCreate(RdbStore &store)
{
    INTELL_VOICE_LOG_DEBUG("OnCreate");
    std::string CREATE_TABLE_Trigger =
        std::string("CREATE TABLE IF NOT EXISTS trigger ") +
        std::string("(model_uuid INTEGER PRIMARY KEY, vendor_uuid INTEGER, data BLOB, model_version INTEGER)");

    return store.ExecuteSql(CREATE_TABLE_Trigger);
}

int TriggerModelOpenCallback::OnUpgrade(RdbStore &store, int oldVersion, int newVersion)
{
    return E_OK;
}

TriggerDbHelper::TriggerDbHelper()
{
    int errCode = E_OK;
    RdbStoreConfig config("/data/service/el1/public/database/intell_voice_service_manager/triggerModel.db");
    TriggerModelOpenCallback helper;
    store_ = RdbHelper::GetRdbStore(config, 1, helper, errCode);
    if (store_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("store is nullptr");
    }
}

TriggerDbHelper::~TriggerDbHelper()
{
    store_ = nullptr;
}

bool TriggerDbHelper::UpdateGenericTriggerModel(std::shared_ptr<GenericTriggerModel> model)
{
    std::lock_guard<std::mutex> lock(mutex_);
    INTELL_VOICE_LOG_INFO("db Update");

    if (store_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("store is nullptr");
        return false;
    }

    if (model == nullptr) {
        INTELL_VOICE_LOG_ERROR("model is nullptr");
        return false;
    }

    int64_t rowId = -1;
    ValuesBucket values;
    values.PutInt("model_uuid", model->GetUuid());
    values.PutInt("vendor_uuid", model->GetVendorUuid());
    values.PutBlob("data", model->GetData());
    values.PutInt("model_version", model->GetVersion());
    int ret = store_->InsertWithConflictResolution(rowId, "trigger", values, ConflictResolution::ON_CONFLICT_REPLACE);
    if (ret != E_OK) {
        INTELL_VOICE_LOG_ERROR("update generic model failed");
        return false;
    }
    return true;
}

bool TriggerDbHelper::GetVendorUuid(std::shared_ptr<AbsSharedResultSet> &set, int32_t &vendorUuid)
{
    int columnIndex;
    int ret = set->GetColumnIndex("vendor_uuid", columnIndex);
    if (ret != E_OK) {
        INTELL_VOICE_LOG_ERROR("failed to get model uuid column index");
        return false;
    }

    ret = set->GetInt(columnIndex, vendorUuid);
    if (ret != E_OK) {
        INTELL_VOICE_LOG_ERROR("failed to get model uuid");
        return false;
    }
    return true;
}

bool TriggerDbHelper::GetBlob(std::shared_ptr<AbsSharedResultSet> &set, std::vector<uint8_t> &data)
{
    int columnIndex;
    int ret = set->GetColumnIndex("data", columnIndex);
    if (ret != E_OK) {
        INTELL_VOICE_LOG_ERROR("failed to get data column index");
        return false;
    }

    ret = set->GetBlob(columnIndex, data);
    if (ret != E_OK) {
        INTELL_VOICE_LOG_ERROR("failed to get data");
        return false;
    }
    return true;
}

bool TriggerDbHelper::GetModelVersion(std::shared_ptr<AbsSharedResultSet> &set, int32_t &version)
{
    int columnIndex;
    int ret = set->GetColumnIndex("model_version", columnIndex);
    if (ret != E_OK) {
        INTELL_VOICE_LOG_ERROR("failed to get model uuid column index");
        return false;
    }

    ret = set->GetInt(columnIndex, version);
    if (ret != E_OK) {
        INTELL_VOICE_LOG_ERROR("failed to get model uuid");
        return false;
    }
    return true;
}

std::shared_ptr<GenericTriggerModel> TriggerDbHelper::GetGenericTriggerModel(const int32_t modelUuid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    INTELL_VOICE_LOG_INFO("db get generic model");
    if (store_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("store is nullptr");
        return nullptr;
    }

    std::shared_ptr<AbsSharedResultSet> set = store_->QuerySql(
        "SELECT * FROM trigger WHERE model_uuid = ?", std::vector<std::string> {std::to_string(modelUuid)});
    if (set == nullptr) {
        INTELL_VOICE_LOG_ERROR("set is nullptr");
        return nullptr;
    }

    set->GoToFirstRow();

    int32_t vendorUuid;
    if (!GetVendorUuid(set, vendorUuid)) {
        INTELL_VOICE_LOG_ERROR("failed to get vendor uuid");
        return nullptr;
    }

    std::vector<uint8_t> data;
    if (!GetBlob(set, data)) {
        INTELL_VOICE_LOG_ERROR("failed to get data");
        return nullptr;
    }

    int32_t modelVersion;
    if (!GetModelVersion(set, modelVersion)) {
        INTELL_VOICE_LOG_ERROR("failed to get model version");
        return nullptr;
    }

    std::shared_ptr<GenericTriggerModel> model = std::make_shared<GenericTriggerModel>(modelUuid, modelVersion);
    if (model == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to alloc model");
        return nullptr;
    }
    model->SetData(data);
    return model;
}

void TriggerDbHelper::DeleteGenericTriggerModel(const int32_t modelUuid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    INTELL_VOICE_LOG_INFO("db Delete");
    if (store_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("store is nullptr");
        return;
    }
    int deletedRows;
    store_->Delete(deletedRows, "trigger", "model_uuid = ?", std::vector<std::string> {std::to_string(modelUuid)});
}
}  // namespace IntellVoiceTrigger
}  // namespace OHOS
