/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2023-2023.
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
#ifndef WHISPER_UPDATE_STRATEGY_H
#define WHISPER_UPDATE_STRATEGY_H

#include "update_strategy.h"

namespace OHOS {
namespace IntellVoiceEngine {
class WhisperUpdateStrategy final: public IUpdateStrategy {
public:
    ~WhisperUpdateStrategy() override;
    explicit WhisperUpdateStrategy(const std::string &param);
    bool UpdateRestrain() override;
    UpdatePriority GetUpdatePriority() override;
    int GetRetryTimes() override;
    int OnUpdateCompleteCallback(const int result, bool isLast) override;
private:
    static void NotifyUpdateFail();
};
}
}
#endif
