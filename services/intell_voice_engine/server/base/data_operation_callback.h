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
#ifndef DATA_OPERATION_CALLBACK_H
#define DATA_OPERATION_CALLBACK_H

#include <ashmem.h>
#include <memory>
#include <string>
#include "v1_1/iintell_voice_data_opr_callback.h"
#include "array_buffer_util.h"

namespace OHOS {
namespace IntellVoiceEngine {
using OHOS::HDI::IntelligentVoice::Engine::V1_1::IntellVoiceDataOprType;
using OHOS::HDI::IntelligentVoice::Engine::V1_1::IIntellVoiceDataOprCallback;

class DataOperationCallback final : public IIntellVoiceDataOprCallback {
public:
    DataOperationCallback() = default;
    ~DataOperationCallback() = default;
    int32_t OnIntellVoiceDataOprEvent(IntellVoiceDataOprType type, const sptr<Ashmem> &inBuffer,
        sptr<Ashmem> &outBuffer) override;

private:
    std::unique_ptr<OHOS::IntellVoiceUtils::Uint8ArrayBuffer> CreateArrayBufferFromAshmem(const sptr<Ashmem> &ashmem);
    sptr<Ashmem> CreateAshmemFromArrayBuffer(const std::unique_ptr<OHOS::IntellVoiceUtils::Uint8ArrayBuffer> &buffer,
        const std::string &name);
};
}
}
#endif