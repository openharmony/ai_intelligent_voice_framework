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
#ifndef HUKS_AES_ADAPTER_H
#define HUKS_AES_ADAPTER_H

#include "hks_param.h"
#include "hks_type.h"
#include "array_buffer_util.h"

namespace OHOS {
namespace IntellVoiceUtils {
class HuksAesAdapter {
public:
    HuksAesAdapter() = default;
    ~HuksAesAdapter() = default;

    static int32_t Encrypt(std::unique_ptr<Uint8ArrayBuffer> &inBuffer, std::unique_ptr<Uint8ArrayBuffer> &outBuffer);
    static int32_t Decrypt(std::unique_ptr<Uint8ArrayBuffer> &inBuffer, std::unique_ptr<Uint8ArrayBuffer> &outBuffer);
private:

    static int32_t IsKeyExist(struct HksBlob *keyAlias, bool &isExist);
    static int32_t GenerateKey(struct HksBlob *keyAlias);
    static int32_t ConstructParamSet(struct HksParamSet **paramSet, const struct HksParam *params,
        uint32_t paramCount);
    static int32_t CreateEncryptParamSet(struct HksParamSet **encryptParamSet, struct HksBlob *encryptNonce);
    static int32_t CreateDecryptParamSet(struct HksParamSet **decryptParamSet, struct HksBlob *decryptNonce,
        struct HksBlob *decryptAead);
    static int32_t UpdateAndFinish(const struct HksBlob *handle, const struct HksParamSet *paramSet,
        const struct HksBlob *inData, struct HksBlob *outData);
};
}
}
#endif