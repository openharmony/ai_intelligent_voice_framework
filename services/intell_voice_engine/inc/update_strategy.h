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
#ifndef UPDATE_STRATEGY_H
#define UPDATE_STRATEGY_H

#include <string>
#include "update_state.h"

namespace OHOS {
namespace IntellVoiceEngine {

class IUpdateStrategy {
public:
    explicit IUpdateStrategy(const std::string &param) : param_(param) {};
    virtual ~IUpdateStrategy() = default;
    virtual bool UpdateRestrain() = 0;
    virtual UpdatePriority GetUpdatePriority() = 0;
    virtual int GetRetryTimes() = 0;
    virtual int OnUpdateCompleteCallback(const int result, bool isLast) = 0;
    std::string param_;
};
}
}
#endif
