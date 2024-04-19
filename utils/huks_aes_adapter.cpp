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
#include "huks_aes_adapter.h"

#include <string>
#include "hks_api.h"
#include "securec.h"
#include "intell_voice_log.h"
#include "scope_guard.h"

#define LOG_TAG "HuksAesAdapter"

using namespace std;

namespace OHOS {
namespace IntellVoiceUtils {
static char g_aliasName[] = "IntelligentVoiceKey";
static char g_aadValue[] = "IntelligentVoiceAAD";

static constexpr uint32_t MAX_UPDATE_SIZE = 64 * 1024;
static constexpr uint32_t MAX_OUTDATA_SIZE = 128 * 1024;

static constexpr uint32_t NONCE_SIZE = 12;
static constexpr uint32_t AEAD_SIZE = 16;

static struct HksParam g_existParams[] = {
    {
        .tag = HKS_TAG_AUTH_STORAGE_LEVEL,
        .uint32Param = HKS_AUTH_STORAGE_LEVEL_DE
    }
};

static struct HksParam g_genParams[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_AES
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_AES_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_GCM
    }, {
        .tag = HKS_TAG_AUTH_STORAGE_LEVEL,
        .uint32Param = HKS_AUTH_STORAGE_LEVEL_DE
    }
};

int32_t HuksAesAdapter::CreateEncryptParamSet(struct HksParamSet **encryptParamSet, struct HksBlob *encryptNonce)
{
    int32_t ret = HksGenerateRandom(nullptr, encryptNonce);
    if (ret != HKS_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("generate random encrypt nonce failed, ret:%{public}d", ret);
        return ret;
    }

    struct HksParam encryptParams[] = {
        {
            .tag = HKS_TAG_ALGORITHM,
            .uint32Param = HKS_ALG_AES
        }, {
            .tag = HKS_TAG_PURPOSE,
            .uint32Param = HKS_KEY_PURPOSE_ENCRYPT
        }, {
            .tag = HKS_TAG_KEY_SIZE,
            .uint32Param = HKS_AES_KEY_SIZE_128
        }, {
            .tag = HKS_TAG_PADDING,
            .uint32Param = HKS_PADDING_NONE
        }, {
            .tag = HKS_TAG_BLOCK_MODE,
            .uint32Param = HKS_MODE_GCM
        }, {
            .tag = HKS_TAG_DIGEST,
            .uint32Param = HKS_DIGEST_NONE
        }, {
            .tag = HKS_TAG_ASSOCIATED_DATA,
            .blob = {
                .size = static_cast<uint32_t>(strlen(g_aadValue)),
                .data = reinterpret_cast<uint8_t*>(g_aadValue)
            }
        }, {
            .tag = HKS_TAG_NONCE,
            .blob = {
                .size = encryptNonce->size,
                .data = encryptNonce->data
            }
        }, {
            .tag = HKS_TAG_AUTH_STORAGE_LEVEL,
            .uint32Param = HKS_AUTH_STORAGE_LEVEL_DE
        }
    };
    ret = ConstructParamSet(encryptParamSet, encryptParams, sizeof(encryptParams) / sizeof(struct HksParam));
    if (ret != HKS_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("constuct encrypt param set failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t HuksAesAdapter::CreateDecryptParamSet(struct HksParamSet **decryptParamSet, struct HksBlob *decryptNonce,
    struct HksBlob *decryptAead)
{
    struct HksParam decryptParams[] = {
        {
            .tag = HKS_TAG_ALGORITHM,
            .uint32Param = HKS_ALG_AES
        }, {
            .tag = HKS_TAG_PURPOSE,
            .uint32Param = HKS_KEY_PURPOSE_DECRYPT
        }, {
            .tag = HKS_TAG_KEY_SIZE,
            .uint32Param = HKS_AES_KEY_SIZE_128
        }, {
            .tag = HKS_TAG_PADDING,
            .uint32Param = HKS_PADDING_NONE
        }, {
            .tag = HKS_TAG_BLOCK_MODE,
            .uint32Param = HKS_MODE_GCM
        }, {
            .tag = HKS_TAG_DIGEST,
            .uint32Param = HKS_DIGEST_NONE
        }, {
            .tag = HKS_TAG_ASSOCIATED_DATA,
            .blob = {
                .size = static_cast<uint32_t>(strlen(g_aadValue)),
                .data = reinterpret_cast<uint8_t*>(g_aadValue)
            }
        }, {
            .tag = HKS_TAG_NONCE,
            .blob = {
                .size = decryptNonce->size,
                .data = decryptNonce->data
            }
        }, {
            .tag = HKS_TAG_AE_TAG,
            .blob = {
                .size = decryptAead->size,
                .data = decryptAead->data
            }
        }, {
            .tag = HKS_TAG_AUTH_STORAGE_LEVEL,
            .uint32Param = HKS_AUTH_STORAGE_LEVEL_DE
        }
    };

    int32_t ret = ConstructParamSet(decryptParamSet, decryptParams, sizeof(decryptParams) / sizeof(struct HksParam));
    if (ret != HKS_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("constuct decrypt param set failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t HuksAesAdapter::Encrypt(std::unique_ptr<Uint8ArrayBuffer> &inBuffer,
    std::unique_ptr<Uint8ArrayBuffer> &outBuffer)
{
    CHECK_CONDITION_RETURN_RET(((inBuffer == nullptr) || (inBuffer->GetSize() == 0)), HKS_ERROR_INVALID_ARGUMENT,
        "invalid arguments");
    struct HksBlob keyAlias = { static_cast<uint32_t>(strlen(g_aliasName)), reinterpret_cast<uint8_t *>(g_aliasName) };
    bool isExist = false;
    int32_t ret = IsKeyExist(&keyAlias, isExist);
    CHECK_CONDITION_RETURN_RET((ret != HKS_SUCCESS), ret, "key exist failed");
    if (!isExist) {
        ret = GenerateKey(&keyAlias);
        CHECK_CONDITION_RETURN_RET((ret != HKS_SUCCESS), ret, "generate key failed");
    }

    struct HksParamSet *encryptParamSet = nullptr;
    uint8_t nonceValue[NONCE_SIZE] = { 0 };
    struct HksBlob encryptNonce = { NONCE_SIZE, nonceValue};
    ret = CreateEncryptParamSet(&encryptParamSet, &encryptNonce);
    CHECK_CONDITION_RETURN_RET((ret != HKS_SUCCESS), ret, "create encrypt param set failed");

    ON_SCOPE_EXIT {
        HksFreeParamSet(&encryptParamSet);
    };

    uint8_t handle[sizeof(uint64_t)] = {0};
    struct HksBlob handleEncrypt = { sizeof(uint64_t), handle };
    ret = HksInit(&keyAlias, encryptParamSet, &handleEncrypt, nullptr);
    if (ret != HKS_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("huks init failed, ret:%{public}d", ret);
        return ret;
    }

    struct HksBlob inData = { inBuffer->GetSize() * sizeof(uint8_t), inBuffer->GetData() };
    auto encryptData = std::make_unique<uint8_t[]>(inBuffer->GetSize() * sizeof(uint8_t) + NONCE_SIZE + AEAD_SIZE);
    CHECK_CONDITION_RETURN_RET((encryptData == nullptr), HKS_ERROR_MALLOC_FAIL, "encryptData is nullptr");

    struct HksBlob outData = { inBuffer->GetSize() * sizeof(uint8_t) + AEAD_SIZE, encryptData.get() + NONCE_SIZE };

    ret = UpdateAndFinish(&handleEncrypt, encryptParamSet, &inData, &outData);
    CHECK_CONDITION_RETURN_RET((ret != HKS_SUCCESS), ret, "update and finish failed");

    (void)memcpy_s(encryptData.get(), NONCE_SIZE, encryptNonce.data, encryptNonce.size);
    outBuffer = CreateArrayBuffer<uint8_t>(outData.size + NONCE_SIZE, encryptData.get());
    if (outBuffer == nullptr) {
        INTELL_VOICE_LOG_ERROR("allocate out buffer failed, size:%{public}u", outData.size);
        return HKS_ERROR_MALLOC_FAIL;
    }
    return HKS_SUCCESS;
}

int32_t HuksAesAdapter::Decrypt(std::unique_ptr<Uint8ArrayBuffer> &inBuffer,
    std::unique_ptr<Uint8ArrayBuffer> &outBuffer)
{
    if ((inBuffer == nullptr) || (inBuffer->GetSize() <= (NONCE_SIZE + AEAD_SIZE))) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    struct HksBlob keyAlias = { static_cast<uint32_t>(strlen(g_aliasName)), reinterpret_cast<uint8_t *>(g_aliasName) };
    bool isExist = false;
    int32_t ret = IsKeyExist(&keyAlias, isExist);
    CHECK_CONDITION_RETURN_RET((ret != HKS_SUCCESS), ret, "key exist failed");
    CHECK_CONDITION_RETURN_RET((!isExist), HKS_FAILURE, "key is not exist");

    struct HksParamSet *decryptParamSet = nullptr;
    struct HksBlob decryptNonce = { NONCE_SIZE, inBuffer->GetData() };
    struct HksBlob decryptAead = { AEAD_SIZE, inBuffer->GetData() + inBuffer->GetSize() * sizeof(uint8_t) - AEAD_SIZE};
    ret = CreateDecryptParamSet(&decryptParamSet, &decryptNonce, &decryptAead);
    CHECK_CONDITION_RETURN_RET((ret != HKS_SUCCESS), ret, "create decrypt param set failed");

    ON_SCOPE_EXIT {
        HksFreeParamSet(&decryptParamSet);
    };

    uint8_t handle[sizeof(uint64_t)] = { 0 };
    struct HksBlob handleDecrypt = { sizeof(uint64_t), handle };
    ret = HksInit(&keyAlias, decryptParamSet, &handleDecrypt, nullptr);
    if (ret != HKS_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("huks init failed, ret:%{public}d", ret);
        return ret;
    }

    auto decryptData = std::make_unique<uint8_t[]>(inBuffer->GetSize() * sizeof(uint8_t) - NONCE_SIZE - AEAD_SIZE);
    CHECK_CONDITION_RETURN_RET((decryptData == nullptr), HKS_ERROR_MALLOC_FAIL, "decryptData is nullptr");

    struct HksBlob inData = { inBuffer->GetSize() * sizeof(uint8_t) - NONCE_SIZE - AEAD_SIZE,
        inBuffer->GetData() + NONCE_SIZE };
    struct HksBlob outData = { inBuffer->GetSize() * sizeof(uint8_t) - NONCE_SIZE - AEAD_SIZE, decryptData.get() };

    ret = UpdateAndFinish(&handleDecrypt, decryptParamSet, &inData, &outData);
    if (ret != HKS_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("update and finish failed, size:%{public}u", outData.size);
        return ret;
    }

    outBuffer = CreateArrayBuffer<uint8_t>(outData.size, outData.data);
    if (outBuffer == nullptr) {
        INTELL_VOICE_LOG_ERROR("allocate out buffer failed, size:%{public}u", outData.size);
        return HKS_ERROR_MALLOC_FAIL;
    }
    return HKS_SUCCESS;
}

int32_t HuksAesAdapter::IsKeyExist(struct HksBlob *keyAlias, bool &isExist)
{
    struct HksParamSet *existParamSet = nullptr;
    int32_t ret = ConstructParamSet(&existParamSet, g_existParams, sizeof(g_existParams) / sizeof(struct HksParam));
    if (ret != HKS_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("constuct exist param set failed");
        return ret;
    }

    if (HksKeyExist(keyAlias, existParamSet) == HKS_SUCCESS) {
        INTELL_VOICE_LOG_INFO("key is already exist");
        isExist = true;
    } else {
        INTELL_VOICE_LOG_INFO("key is not exist");
        isExist = false;
    }

    HksFreeParamSet(&existParamSet);
    return HKS_SUCCESS;
}

int32_t HuksAesAdapter::GenerateKey(struct HksBlob *keyAlias)
{
    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = ConstructParamSet(&genParamSet, g_genParams, sizeof(g_genParams) / sizeof(struct HksParam));
    if (ret != HKS_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("constuct gen param set failed");
        return ret;
    }

    ret = HksGenerateKey(keyAlias, genParamSet, nullptr);
    if (ret != HKS_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("generate key failed, ret:%{public}d", ret);
    }

    HksFreeParamSet(&genParamSet);
    return ret;
}

int32_t HuksAesAdapter::ConstructParamSet(struct HksParamSet **paramSet, const struct HksParam *params,
    uint32_t paramCount)
{
    int32_t ret = HksInitParamSet(paramSet);
    if (ret != HKS_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("HksInitParamSet failed, ret:%{public}d", ret);
        return ret;
    }

    ON_SCOPE_EXIT {
        HksFreeParamSet(paramSet);
    };

    ret = HksAddParams(*paramSet, params, paramCount);
    if (ret != HKS_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("HksAddParams failed, ret:%{public}d", ret);
        return ret;
    }

    ret = HksBuildParamSet(paramSet);
    if (ret != HKS_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("HksBuildParamSet failed, ret:%{public}d", ret);
        return ret;
    }

    CANCEL_SCOPE_EXIT;
    return ret;
}

int32_t HuksAesAdapter::UpdateAndFinish(const struct HksBlob *handle, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData)
{
    struct HksBlob inDataSeg = { MAX_UPDATE_SIZE, inData->data };
    struct HksBlob outDataSeg = { MAX_OUTDATA_SIZE, nullptr };
    uint32_t inUpdateSize = 0;
    uint8_t *outDataAddr = outData->data;
    uint32_t outUpdateSize = 0;
    int32_t ret = HKS_SUCCESS;

    while (inUpdateSize < inData->size) {
        if (inUpdateSize + MAX_UPDATE_SIZE >= inData->size) {
            inDataSeg.size = inData->size - inUpdateSize;
            break;
        }

        std::unique_ptr<Uint8ArrayBuffer> tmpUpdateBuff= CreateArrayBuffer<uint8_t>(outDataSeg.size);
        CHECK_CONDITION_RETURN_RET((tmpUpdateBuff == nullptr), HKS_FAILURE, "tmpUpdateBuff is nullptr");
        outDataSeg.data = tmpUpdateBuff->GetData();

        ret = HksUpdate(handle, paramSet, &inDataSeg, &outDataSeg);
        if (ret != HKS_SUCCESS) {
            INTELL_VOICE_LOG_ERROR("Hks update failed, ret:%{public}d", ret);
            return HKS_FAILURE;
        }

        if (outUpdateSize + outDataSeg.size > outData->size) {
            INTELL_VOICE_LOG_ERROR("Hks update size failed, outUpdateSize:%{public}u", outUpdateSize);
            return HKS_FAILURE;
        }

        (void)memcpy_s(outDataAddr, outData->size - outUpdateSize, outDataSeg.data, outDataSeg.size);
        outDataAddr += outDataSeg.size;
        outUpdateSize += outDataSeg.size;
        tmpUpdateBuff = nullptr;
        inUpdateSize += MAX_UPDATE_SIZE;
        inDataSeg.data = inData->data + inUpdateSize;
    }

    std::unique_ptr<Uint8ArrayBuffer> tmpFinishBuff= CreateArrayBuffer<uint8_t>(inDataSeg.size + AEAD_SIZE);
    CHECK_CONDITION_RETURN_RET((tmpFinishBuff == nullptr), HKS_FAILURE, "tmpFinishBuff is nullptr");

    struct HksBlob outDataFinish = { tmpFinishBuff->GetSize() * sizeof(uint8_t), tmpFinishBuff->GetData() };
    ret = HksFinish(handle, paramSet, &inDataSeg, &outDataFinish);
    if (ret != HKS_SUCCESS) {
        INTELL_VOICE_LOG_ERROR("Hks finish failed, ret:%{public}d", ret);
        return HKS_FAILURE;
    }

    if (outUpdateSize + outDataFinish.size > outData->size) {
        INTELL_VOICE_LOG_ERROR("%{public}u, %{public}u, %{public}u", outUpdateSize, outDataFinish.size, outData->size);
        return HKS_FAILURE;
    }
    (void)memcpy_s(outDataAddr, outData->size - outUpdateSize, outDataFinish.data, outDataFinish.size);
    outData->size = outUpdateSize + outDataFinish.size;

    return HKS_SUCCESS;
}
}
}
