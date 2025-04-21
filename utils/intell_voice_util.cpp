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
#include "intell_voice_util.h"
#include <sys/stat.h>
#include <memory>
#include <malloc.h>
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <cinttypes>
#include <fstream>
#include "string_util.h"
#include "accesstoken_kit.h"
#include "privacy_kit.h"
#include "tokenid_kit.h"
#include "ipc_skeleton.h"
#include "privacy_error.h"
#include "intell_voice_log.h"
#include "intell_voice_info.h"
#include "ability_manager_client.h"
#include "history_info_mgr.h"

#define LOG_TAG "IntellVoiceUtil"

using namespace OHOS::IntellVoice;

namespace OHOS {
namespace IntellVoiceUtils {
static constexpr uint32_t VERSION_OFFSET = 8;

uint32_t IntellVoiceUtil::GetHdiVersionId(uint32_t majorVer, uint32_t minorVer)
{
    return ((majorVer << VERSION_OFFSET) | minorVer);
}

bool IntellVoiceUtil::DeinterleaveAudioData(const int16_t *buffer, uint32_t size, int32_t channelCnt,
    std::vector<std::vector<uint8_t>> &audioData)
{
    if (channelCnt == 0) {
        INTELL_VOICE_LOG_ERROR("channel cnt is zero");
        return false;
    }
    uint32_t channelLen = size / channelCnt;
    std::unique_ptr<int16_t[]> channelData = std::make_unique<int16_t[]>(channelLen);
    if (channelData == nullptr) {
        INTELL_VOICE_LOG_ERROR("channelData is nullptr");
        return false;
    }
    for (int32_t i = 0; i < channelCnt; i++) {
        for (uint32_t j = 0; j < channelLen; j++) {
            channelData[j] = buffer[i + j * channelCnt];
        }
        std::vector<uint8_t> item(reinterpret_cast<uint8_t *>(channelData.get()),
            reinterpret_cast<uint8_t *>(channelData.get()) + channelLen * sizeof(int16_t));
        audioData.emplace_back(item);
    }
    return true;
}

bool IntellVoiceUtil::ReadFile(const std::string &filePath, std::shared_ptr<uint8_t> &buffer, uint32_t &size)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.good()) {
        INTELL_VOICE_LOG_ERROR("open file failed");
        return false;
    }

    file.seekg(0, file.end);
    size = static_cast<uint32_t>(file.tellg());
    if (size == 0) {
        INTELL_VOICE_LOG_ERROR("file is empty");
        return false;
    }
    buffer = std::shared_ptr<uint8_t>(new uint8_t[size], [](uint8_t *p) { delete[] p; });
    if (buffer == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to allocate buffer");
        return false;
    }

    file.seekg(0, file.beg);
    file.read(reinterpret_cast<char *>(buffer.get()), size);
    file.close();
    return true;
}

void IntellVoiceUtil::SplitStringToKVPair(const std::string &inputStr, std::map<std::string, std::string> &kvpairs)
{
    std::vector<std::string> paramsList;
    StringUtil::Split(inputStr, ";", paramsList);
    for (const auto &it : paramsList) {
        std::string key;
        std::string value;
        if (StringUtil::SplitLineToPair(it, key, value)) {
            kvpairs[key] = value;
            INTELL_VOICE_LOG_INFO("key:%{public}s, value:%{public}s", key.c_str(), value.c_str());
        }
    }
}

bool IntellVoiceUtil::IsFileExist(const std::string &filePath)
{
    struct stat sb;
    if (stat(filePath.c_str(), &sb) != 0) {
        INTELL_VOICE_LOG_ERROR("get file status failed");
        return false;
    }

    return true;
}

bool IntellVoiceUtil::VerifyClientPermission(const std::string &permissionName)
{
    Security::AccessToken::AccessTokenID clientTokenId = IPCSkeleton::GetCallingTokenID();
    INTELL_VOICE_LOG_INFO("clientTokenId:%{public}d, permission name:%{public}s", clientTokenId,
        permissionName.c_str());
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(clientTokenId, permissionName);
    if (res != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        INTELL_VOICE_LOG_ERROR("Permission denied");
        return false;
    }
    return true;
}

bool IntellVoiceUtil::CheckIsSystemApp()
{
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(IPCSkeleton::GetCallingTokenID()) ==
        Security::AccessToken::TOKEN_NATIVE) {
        INTELL_VOICE_LOG_INFO("calling by native");
        return true;
    }

    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        INTELL_VOICE_LOG_INFO("Not system app, permission reject tokenid: %{public}" PRIu64 "", fullTokenId);
        return false;
    }

    INTELL_VOICE_LOG_INFO("System app, fullTokenId:%{public}" PRIu64 "", fullTokenId);
    return true;
}

int32_t IntellVoiceUtil::VerifySystemPermission(const std::string &permissionName)
{
#ifdef INTELL_VOICE_BUILD_VARIANT_ROOT
    if (IPCSkeleton::GetCallingUid() == 0) { // 0 for root uid
        INTELL_VOICE_LOG_INFO("callingUid is root");
        return INTELLIGENT_VOICE_SUCCESS;
    }
#endif

    if (!CheckIsSystemApp()) {
        return INTELLIGENT_VOICE_NOT_SYSTEM_APPLICATION;
    }

    if (!VerifyClientPermission(permissionName)) {
        return INTELLIGENT_VOICE_PERMISSION_DENIED;
    }
    return INTELLIGENT_VOICE_SUCCESS;
}

bool IntellVoiceUtil::RecordPermissionPrivacy(const std::string &permissionName, uint32_t targetTokenId,
    IntellVoicePermissionState state)
{
    INTELL_VOICE_LOG_INFO("permissionName:%{public}s, tokenId:%{public}u, state:%{public}d", permissionName.c_str(),
        targetTokenId, state);
    if (state == INTELL_VOICE_PERMISSION_START) {
        auto ret = Security::AccessToken::PrivacyKit::StartUsingPermission(targetTokenId, permissionName);
        if (ret != 0 && ret != Security::AccessToken::ERR_PERMISSION_ALREADY_START_USING) {
            INTELL_VOICE_LOG_ERROR("StartUsingPermission for tokenId %{public}u, ret is %{public}d",
                targetTokenId, ret);
            return false;
        }
        ret = Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(targetTokenId, permissionName, 1, 0);
        if (ret != 0 && ret != Security::AccessToken::ERR_PERMISSION_ALREADY_START_USING) {
            INTELL_VOICE_LOG_ERROR("AddPermissionUsedRecord for tokenId %{public}u! The PrivacyKit error code is "
                "%{public}d", targetTokenId, ret);
            return false;
        }
    } else if (state == INTELL_VOICE_PERMISSION_STOP) {
        auto ret = Security::AccessToken::PrivacyKit::StopUsingPermission(targetTokenId, permissionName);
        if (ret != 0) {
            INTELL_VOICE_LOG_ERROR("StopUsingPermission for tokenId %{public}u, ret is %{public}d",
                targetTokenId, ret);
            return false;
        }
    } else {
        INTELL_VOICE_LOG_WARN("invalid state");
        return false;
    }

    return true;
}

void IntellVoiceUtil::StartAbility(const std::string &event)
{
    AAFwk::Want want;
    HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();

    std::string bundleName = historyInfoMgr.GetStringKVPair(KEY_WAKEUP_ENGINE_BUNDLE_NAME);
    std::string abilityName = historyInfoMgr.GetStringKVPair(KEY_WAKEUP_ENGINE_ABILITY_NAME);
    INTELL_VOICE_LOG_INFO("bundleName:%{public}s, abilityName:%{public}s", bundleName.c_str(), abilityName.c_str());
    if (bundleName.empty() || abilityName.empty()) {
        INTELL_VOICE_LOG_ERROR("bundle name is empty or ability name is empty");
        return;
    }
    want.SetElementName(bundleName, abilityName);
    want.SetParam("serviceName", std::string("intell_voice"));
    want.SetParam("servicePid", getpid());
    want.SetParam("eventType", event);
    auto abilityManagerClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityManagerClient == nullptr) {
        INTELL_VOICE_LOG_ERROR("abilityManagerClient is nullptr");
        return;
    }
    abilityManagerClient->StartAbility(want);
}
}
}
