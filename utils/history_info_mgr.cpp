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
#include "history_info_mgr.h"

#include "string_util.h"

#define LOG_TAG "HistoryInfoMgr"

namespace OHOS {
namespace IntellVoiceUtils {
constexpr int DECIMAL_NOTATION = 10;

HistoryInfoMgr::HistoryInfoMgr()
    : ServiceDbHelper("intell_voice_service_manager", "local_intell_voice_history_mgr_storeId")
{
}

void HistoryInfoMgr::SetIntKVPair(std::string key, int32_t value)
{
    SetValue(key, StringUtil::Int2String(value));
}

int32_t HistoryInfoMgr::GetIntKVPair(std::string key)
{
    std::string value = GetValue(key);
    return static_cast<int32_t>(strtol(value.c_str(), nullptr, DECIMAL_NOTATION));
}

void HistoryInfoMgr::SetStringKVPair(std::string key, std::string value)
{
    SetValue(key, value);
}

std::string HistoryInfoMgr::GetStringKVPair(std::string key)
{
    return GetValue(key);
}

void HistoryInfoMgr::DeleteKey(const std::vector<std::string> &keyList)
{
    for (auto key : keyList) {
        Delete(key);
    }
}
}
}