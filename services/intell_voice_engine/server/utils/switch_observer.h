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
#ifndef SWITCH_OBSERVER_H
#define SWITCH_OBSERVER_H

#include <functional>
#include "data_ability_observer_stub.h"

namespace OHOS {
namespace IntellVoiceEngine {
using UpdateFunc = std::function<void()>;

class SwitchObserver : public AAFwk::DataAbilityObserverStub {
public:
    SwitchObserver() = default;
    virtual ~SwitchObserver() = default;
    void OnChange() override;
    void SetUpdateFunc(UpdateFunc func);

private:
    UpdateFunc update_ = nullptr;
};
}  // namespace IntellVoice
}  // namespace OHOS
#endif