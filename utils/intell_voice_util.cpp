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
#include <memory>
#include <malloc.h>
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <cinttypes>
#include <fstream>

#include "accesstoken_kit.h"
#include "tokenid_kit.h"
#include "ipc_skeleton.h"
#include "intell_voice_log.h"

#define LOG_TAG "IntellVoiceUtil"

namespace OHOS {
namespace IntellVoiceUtils {
static constexpr uint32_t VERSION_OFFSET = 8;
static constexpr uid_t UID_ROOT = 0;

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

bool IntellVoiceUtil::VerifyClientPermission(const std::string &permissionName)
{
    Security::AccessToken::AccessTokenID clientTokenId = IPCSkeleton::GetCallingTokenID();
    INTELL_VOICE_LOG_INFO("clientTokenId:%{public}d", clientTokenId);
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(clientTokenId, permissionName);
    if (res != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        INTELL_VOICE_LOG_ERROR("Permission denied!");
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

bool IntellVoiceUtil::VerifySystemPermission(const std::string &permissionName)
{
    if (IPCSkeleton::GetCallingUid() == UID_ROOT) {
        INTELL_VOICE_LOG_INFO("callingUid is root");
        return true;
    }

    if (!CheckIsSystemApp()) {
        return false;
    }

    if (!VerifyClientPermission(permissionName)) {
        return false;
    }

    return true;
}
}
}
