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
#include "switch_provider.h"
#include "intell_voice_log.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

using namespace OHOS::IntellVoiceUtils;
#define LOG_TAG "SwitchProvider"

namespace OHOS {
namespace IntellVoiceEngine {
const std::string SWITCH_URI_PROXY = "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
const std::string SWITCH_KEY = "intell_voice_trigger_enabled";

SwitchProvider::SwitchProvider()
{
}

SwitchProvider::~SwitchProvider()
{
    helper_ = nullptr;
}

bool SwitchProvider::Init()
{
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        INTELL_VOICE_LOG_ERROR("saManager is nullptr");
        return false;
    }
    auto remoteObj = saManager->GetSystemAbility(INTELL_VOICE_SERVICE_ID);
    if (remoteObj == nullptr) {
        INTELL_VOICE_LOG_ERROR("remoteObj is nullptr");
        return false;
    }

    helper_ = DataShare::DataShareHelper::Creator(remoteObj, SWITCH_URI_PROXY);
    if (helper_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("helper_ is nullptr");
        return false;
    }

    return true;
}

void SwitchProvider::RegisterObserver(const sptr<SwitchObserver> &observer)
{
    auto uri = AssembleUri(SWITCH_KEY);
    helper_->RegisterObserver(uri, observer);
}

void SwitchProvider::UnregisterObserver(const sptr<SwitchObserver> &observer)
{
    auto uri = AssembleUri(SWITCH_KEY);
    helper_->UnregisterObserver(uri, observer);
}

bool SwitchProvider::QuerySwitchStatus()
{
    std::vector<std::string> columns = {"VALUE"};
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("KEYWORD", SWITCH_KEY);
    auto uri = AssembleUri(SWITCH_KEY);
    auto resultSet = helper_->Query(uri, predicates, columns);
    if (resultSet == nullptr) {
        INTELL_VOICE_LOG_ERROR("helper->Query return nullptr");
        return false;
    }

    int32_t count;
    resultSet->GetRowCount(count);
    if (count == 0) {
        INTELL_VOICE_LOG_ERROR("not found value");
        return false;
    }
    const int32_t INDEX = 0;
    resultSet->GoToRow(INDEX);
    std::string value;
    resultSet->GetString(INDEX, value);
    resultSet->Close();

    if (value == "0") {
        return false;
    } else if (value == "1") {
        return true;
    } else {
        return false;
    }
}

Uri SwitchProvider::AssembleUri(const std::string& key)
{
    Uri uri(SWITCH_URI_PROXY + "&key=" + key);
    return uri;
}
}  // namespace IntellVoice
}  // namespace OHOS