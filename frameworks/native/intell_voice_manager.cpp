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

#include "intell_voice_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "intell_voice_log.h"
#include "i_intell_voice_service.h"
#include "intell_voice_service_proxy.h"

using namespace std;
using namespace OHOS::IntellVoiceEngine;
#define LOG_TAG "IntellVoiceManager"

namespace OHOS {
namespace IntellVoice {
static sptr<IIntellVoiceService> g_sProxy = nullptr;

IntellVoiceManager::IntellVoiceManager()
{
    Init();
}

IntellVoiceManager::~IntellVoiceManager()
{
    INTELL_VOICE_LOG_INFO("enter");
}

IntellVoiceManager *IntellVoiceManager::GetInstance()
{
    static IntellVoiceManager manager;
    return &manager;
}

void IntellVoiceManager::Init()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("get sa manager failed");
        return;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(INTELL_VOICE_SERVICE_ID);
    if (object == nullptr) {
        INTELL_VOICE_LOG_ERROR("get system ability failed");
        return;
    }
    g_sProxy = iface_cast<IIntellVoiceService>(object);
    if (g_sProxy == nullptr) {
        INTELL_VOICE_LOG_ERROR("init Service Proxy failed.");
    } else {
        INTELL_VOICE_LOG_INFO("init Service Proxy success.");
    }
}

int32_t IntellVoiceManager::CreateIntellVoiceEngine(IntellVoiceEngineType type, sptr<IIntellVoiceEngine> &inst)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (g_sProxy == nullptr) {
        INTELL_VOICE_LOG_ERROR("IntellVoiceService Proxy is null.");
        return -1;
    }
    return g_sProxy->CreateIntellVoiceEngine(type, inst);
}

int32_t IntellVoiceManager::ReleaseIntellVoiceEngine(IntellVoiceEngineType type)
{
    INTELL_VOICE_LOG_INFO("enter");
    if (g_sProxy == nullptr) {
        INTELL_VOICE_LOG_ERROR("IntellVoiceService Proxy is null.");
        return -1;
    }
    return g_sProxy->ReleaseIntellVoiceEngine(type);
}
}
}