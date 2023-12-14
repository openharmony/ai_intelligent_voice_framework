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
#include "data_operation_callback.h"
#include "intell_voice_log.h"
#include "huks_aes_adapter.h"
#include "scope_guard.h"

#define LOG_TAG "DataOperationCb"

using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {
int32_t DataOperationCallback::OnIntellVoiceDataOprEvent(
    OHOS::HDI::IntelligentVoice::Engine::V1_1::IntellVoiceDataOprType type,
    const sptr<Ashmem> &inBuffer,
    sptr<Ashmem> &outBuffer)
{
    INTELL_VOICE_LOG_INFO("enter, type:%{public}d", type);
    if (inBuffer == nullptr) {
        INTELL_VOICE_LOG_ERROR("in buffer is nullptr");
        return -1;
    }

    ON_SCOPE_EXIT {
        INTELL_VOICE_LOG_INFO("clear ashmem");
        inBuffer->UnmapAshmem();
        inBuffer->CloseAshmem();
    };

    if ((type < OHOS::HDI::IntelligentVoice::Engine::V1_1::ENCRYPT_TYPE) ||
        (type > OHOS::HDI::IntelligentVoice::Engine::V1_1::DECRYPT_TYPE)) {
        INTELL_VOICE_LOG_ERROR("invalid type:%{public}d", type);
        return -1;
    }

    auto inData = CreateArrayBufferFromAshmem(inBuffer);
    if (inData == nullptr) {
        INTELL_VOICE_LOG_ERROR("create in data failed");
        return -1;
    }

    int32_t ret = HKS_SUCCESS;
    std::unique_ptr<OHOS::IntellVoiceUtils::Uint8ArrayBuffer> outData = nullptr;

    if (type == OHOS::HDI::IntelligentVoice::Engine::V1_1::ENCRYPT_TYPE) {
        ret = HuksAesAdapter::Encrypt(inData, outData);
        if (ret != HKS_SUCCESS) {
            INTELL_VOICE_LOG_ERROR("encrypt failed, ret:%{public}d", ret);
            return -1;
        }
        outBuffer = CreateAshmemFromArrayBuffer(outData, "EnryptOutIntellVoiceData");
    } else if (type == OHOS::HDI::IntelligentVoice::Engine::V1_1::DECRYPT_TYPE) {
        ret = HuksAesAdapter::Decrypt(inData, outData);
        if (ret != HKS_SUCCESS) {
            INTELL_VOICE_LOG_ERROR("decrypt failed, ret:%{public}d", ret);
            return -1;
        }
        outBuffer = CreateAshmemFromArrayBuffer(outData, "DeryptOutIntellVoiceData");
    }

    if (outBuffer == nullptr) {
        INTELL_VOICE_LOG_ERROR("out buffer is nullptr");
        return -1;
    }

    INTELL_VOICE_LOG_INFO("exit");
    return 0;
}

std::unique_ptr<OHOS::IntellVoiceUtils::Uint8ArrayBuffer> DataOperationCallback::CreateArrayBufferFromAshmem(
    const sptr<Ashmem> &ashmem)
{
    if (ashmem == nullptr) {
        INTELL_VOICE_LOG_ERROR("ashmem is nullptr");
        return nullptr;
    }

    uint32_t size = static_cast<uint32_t>(ashmem->GetAshmemSize());
    if (size == 0) {
        INTELL_VOICE_LOG_ERROR("size is zero");
        return nullptr;
    }

    if (!ashmem->MapReadOnlyAshmem()) {
        INTELL_VOICE_LOG_ERROR("map ashmem failed");
        return nullptr;
    }

    const uint8_t *mem = static_cast<const uint8_t *>(ashmem->ReadFromAshmem(size, 0));
    if (mem == nullptr) {
        INTELL_VOICE_LOG_ERROR("read from ashmem failed");
        return nullptr;
    }

    return CreateArrayBuffer<uint8_t>(size, mem);
}

sptr<Ashmem> DataOperationCallback::CreateAshmemFromArrayBuffer(
    const std::unique_ptr<OHOS::IntellVoiceUtils::Uint8ArrayBuffer> &buffer, const std::string &name)
{
    if (buffer == nullptr) {
        INTELL_VOICE_LOG_ERROR("buffer is nullptr");
        return nullptr;
    }

    if ((buffer->GetSize() == 0) || (buffer->GetData() == nullptr)) {
        INTELL_VOICE_LOG_ERROR("data is empty");
        return nullptr;
    }

    sptr<Ashmem> ashmem = OHOS::Ashmem::CreateAshmem(name.c_str(), buffer->GetSize() * sizeof(uint8_t));
    if (ashmem == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to create ashmem");
        return nullptr;
    }

    ON_SCOPE_EXIT {
        ashmem->UnmapAshmem();
        ashmem->CloseAshmem();
        ashmem = nullptr;
    };

    if (!ashmem->MapReadAndWriteAshmem()) {
        INTELL_VOICE_LOG_ERROR("failed to map ashmem");
        return nullptr;
    }

    if (!ashmem->WriteToAshmem(buffer->GetData(), buffer->GetSize() * sizeof(uint8_t), 0)) {
        INTELL_VOICE_LOG_ERROR("failed to write ashmem");
        return nullptr;
    }

    CANCEL_SCOPE_EXIT;
    INTELL_VOICE_LOG_INFO("create ashmem success, size:%{public}u",
        static_cast<uint32_t>(buffer->GetSize() * sizeof(uint8_t)));
    return ashmem;
}
}
}
