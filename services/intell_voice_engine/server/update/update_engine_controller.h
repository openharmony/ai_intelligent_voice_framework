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
#ifndef UPDATE_ENGINE_CONTROLLER_H
#define UPDATE_ENGINE_CONTROLLER_H
#include <memory>
#include <string>
#include <atomic>
#include "engine_base.h"
#include "v1_0/iintell_voice_engine_callback.h"
#include "intell_voice_generic_factory.h"

namespace OHOS {
namespace IntellVoiceEngine {
class UpdateEngineController {
public:
    virtual ~UpdateEngineController();
    UpdateEngineController();

    bool IsVersionUpdate();
    void SaveWakeupVesion();
    virtual bool CreateUpdateEngine()
    {
        return false;
    }
    virtual void OnUpdateComplete(int result) {};

public:
    void SetUpdateState(bool result);
    bool GetUpdateState();
    void GetCurWakeupVesion(std::string &versionNumber);
private:
    static std::atomic<bool> isUpdating_;
};
}
}
#endif
