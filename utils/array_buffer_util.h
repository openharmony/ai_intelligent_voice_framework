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
#ifndef ARRAY_BUFFER_UTIL_H
#define ARRAY_BUFFER_UTIL_H

#include <memory>

namespace OHOS {
namespace IntellVoiceUtils {
template<class T>
class ArrayBufferUtil {
public:
    ~ArrayBufferUtil() = default;

    T *GetData()
    {
        if (data_ == nullptr) {
            return nullptr;
        }

        return data_.get();
    }
    uint32_t GetSize() const
    {
        return size_;
    }

    template<typename C>
    friend std::unique_ptr<ArrayBufferUtil<C>> CreateArrayBuffer(uint32_t size, const C *data);

private:
    ArrayBufferUtil() = default;
    bool Init(uint32_t size, const T *data = nullptr);
    std::unique_ptr<T[]> data_ = nullptr;
    uint32_t size_ = 0;
};

using Uint8ArrayBuffer = ArrayBufferUtil<uint8_t>;
template<class T> std::unique_ptr<ArrayBufferUtil<T>> CreateArrayBuffer(uint32_t size, const T *data = nullptr);

}
}
#endif
