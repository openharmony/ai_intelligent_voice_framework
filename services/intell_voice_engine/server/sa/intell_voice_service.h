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
#ifndef INTELL_VOICE_SERVICE_H
#define INTELL_VOICE_SERVICE_H
#include <map>
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "system_ability.h"
#include "intell_voice_service_stub.h"
#include "i_intell_voice_engine.h"
#include "system_event_observer.h"
#include "audio_capturer_source_change_callback.h"

#include "trigger_manager.h"

namespace OHOS {
namespace IntellVoiceEngine {
class IntellVoiceService : public SystemAbility, public IntellVoiceServiceStub {
    DECLARE_SYSTEM_ABILITY(IntellVoiceService);
public:
    explicit IntellVoiceService(int32_t systemAbilityId, bool runOnCreate = true);
    ~IntellVoiceService();
    int32_t CreateIntellVoiceEngine(IntellVoiceEngineType type, sptr<IIntellVoiceEngine> &inst) override;
    int32_t ReleaseIntellVoiceEngine(IntellVoiceEngineType type) override;
    int32_t Dump(int fd, const std::vector<std::u16string> &args) override;

    class PerStateChangeCbCustomizeCallback : public Security::AccessToken::PermStateChangeCallbackCustomize {
    public:
        explicit PerStateChangeCbCustomizeCallback(const Security::AccessToken::PermStateChangeScope &scopeInfo)
            : PermStateChangeCallbackCustomize(scopeInfo) {}
        ~PerStateChangeCbCustomizeCallback() {}

        void PermStateChangeCallback(Security::AccessToken::PermStateChangeInfo& result) override;
    };

protected:
    void OnStart(const SystemAbilityOnDemandReason& startReason) override;
    void OnStop() override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
private:
    void CreateSystemEventObserver();
    bool VerifyClientPermission(const std::string &permissionName);
    void RegisterPermissionCallback(const std::string &permissionName);
    void LoadIntellVoiceHost();
    void UnloadIntellVoiceHost();

private:
    std::shared_ptr<SystemEventObserver> systemEventObserver_ = nullptr;
    OHOS::OnDemandReasonId reasonId_;
    std::shared_ptr<AudioCapturerSourceChangeCallback> audioCapturerSourceChangeCallback_ = nullptr;
};
}
}
#endif
