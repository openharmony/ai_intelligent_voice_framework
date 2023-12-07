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
#include "array_buffer_util.h"
#include "securec.h"
#include "intell_voice_log.h"

#define LOG_TAG "ArrayBufferUtil"

namespace OHOS {
namespace IntellVoiceUtils {
template<class T> bool ArrayBufferUtil<T>::Init(uint32_t size, const T *data)
{
    if (size == 0) {
        INTELL_VOICE_LOG_ERROR("size is zero");
        return false;
    }

    data_ = std::make_unique<T[]>(size);
    if (data_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("allocate data failed");
        return false;
    }

    if (data == nullptr) {
        (void)memset_s(data_.get(), size * sizeof(T), 0, size * sizeof(T));
    } else {
        (void)memcpy_s(data_.get(), size * sizeof(T), data, size * sizeof(T));
    }

    size_  = size;
    return true;
}

template<class T> std::unique_ptr<ArrayBufferUtil<T>> CreateArrayBuffer(uint32_t size, const T *data)
{
    auto ret = std::unique_ptr<ArrayBufferUtil<T>>(new (std::nothrow) ArrayBufferUtil<T>());
    if (ret == nullptr) {
        INTELL_VOICE_LOG_ERROR("allocate array buffer failed");
        return nullptr;
    }

    if (!ret->Init(size, data)) {
        INTELL_VOICE_LOG_ERROR("init array buffer failed");
        return nullptr;
    }

    return ret;
}

template class ArrayBufferUtil<uint8_t>;
template std::unique_ptr<Uint8ArrayBuffer> CreateArrayBuffer(uint32_t size, const uint8_t *data = nullptr);
}
}