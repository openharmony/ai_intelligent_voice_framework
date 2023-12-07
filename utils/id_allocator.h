/*
 * Copyright (c) 2023 Huawei Device Co., Ltd. 2023-2023.
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
#ifndef ID_ALLOCTOR_H
#define ID_ALLOCTOR_H

#include <vector>

namespace OHOS {
namespace IntellVoiceUtils {
constexpr int INVALID_ID = 0xffff;

struct IdAllocator {
    explicit IdAllocator(int maxTimerNum = 100);
    virtual ~IdAllocator(){};
    int AllocId();
    void ReleaseId(unsigned int id);
    void ClearId();
private:
    std::vector<bool> idFlags;
};
}
}
#endif
