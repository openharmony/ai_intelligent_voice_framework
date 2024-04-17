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
#include "wakeup_engine.h"
#include "ability_manager_client.h"
#include "intell_voice_service_manager.h"
#include "intell_voice_log.h"

#define LOG_TAG "WakeupEngine"

using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {
WakeupEngine::WakeupEngine()
{
    INTELL_VOICE_LOG_INFO("enter");
}

WakeupEngine::~WakeupEngine()
{
    INTELL_VOICE_LOG_INFO("enter");
}

void WakeupEngine::OnDetected(int32_t uuid)
{
    INTELL_VOICE_LOG_INFO("enter, uuid is %{public}d", uuid);
    std::thread([uuid]() { WakeupEngine::StartAbility(uuid); }).detach();
    StateMsg msg(START_RECOGNIZE, &uuid, sizeof(int32_t));
    if (ROLE(WakeupEngineImpl).Handle(msg) != 0) {
        INTELL_VOICE_LOG_WARN("start failed");
        std::thread([]() {
            const auto &manager = IntellVoiceServiceManager::GetInstance();
            if (manager != nullptr) {
                manager->HandleCloseWakeupSource();
            }
        }).detach();
    }
}

bool WakeupEngine::Init(const std::string & /* param */)
{
    StateMsg msg(INIT);
    if (ROLE(WakeupEngineImpl).Handle(msg) != 0) {
        return false;
    }
    return true;
}

void WakeupEngine::SetCallback(sptr<IRemoteObject> object)
{
    sptr<IIntelligentVoiceEngineCallback> callback = iface_cast<IIntelligentVoiceEngineCallback>(object);
    if (callback == nullptr) {
        INTELL_VOICE_LOG_WARN("clear callback");
    }
    SetListenerMsg listenerMsg(callback);
    StateMsg msg(SET_LISTENER, &listenerMsg, sizeof(SetListenerMsg));
    ROLE(WakeupEngineImpl).Handle(msg);
}

int32_t WakeupEngine::Attach(const IntellVoiceEngineInfo & /* info */)
{
    return 0;
}

int32_t WakeupEngine::StartCapturer(int32_t channels)
{
    StateMsg msg(START_CAPTURER, &channels, sizeof(int32_t));
    return ROLE(WakeupEngineImpl).Handle(msg);
}

int32_t WakeupEngine::Read(std::vector<uint8_t> &data)
{
    CapturerData capturerData;
    StateMsg msg(READ, nullptr, 0, reinterpret_cast<void *>(&capturerData));
    int32_t ret = ROLE(WakeupEngineImpl).Handle(msg);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("read failed, ret:%{public}d", ret);
        return -1;
    }

    data.swap(capturerData.data);
    return 0;
}

int32_t WakeupEngine::StopCapturer()
{
    StateMsg msg(STOP_CAPTURER);
    return ROLE(WakeupEngineImpl).Handle(msg);
}

int32_t WakeupEngine::Detach(void)
{
    StateMsg msg(RELEASE);
    return ROLE(WakeupEngineImpl).Handle(msg);
}

int32_t WakeupEngine::Start(bool /* isLast */)
{
    return 0;
}

int32_t WakeupEngine::SetParameter(const std::string &keyValueList)
{
    StringParam param(keyValueList);
    StateMsg msg(SET_PARAM, &param, sizeof(param));
    return ROLE(WakeupEngineImpl).Handle(msg);
}

std::string WakeupEngine::GetParameter(const std::string &key)
{
    StringParam keyParam(key);
    StringParam valueParam;
    StateMsg msg(GET_PARAM, &keyParam, sizeof(keyParam), &valueParam);
    if (ROLE(WakeupEngineImpl).Handle(msg) != 0) {
        return "";
    }

    return valueParam.strParam;
}

int32_t WakeupEngine::Stop()
{
    StateMsg msg(STOP_RECOGNIZE);
    return ROLE(WakeupEngineImpl).Handle(msg);
}

int32_t WakeupEngine::GetWakeupPcm(std::vector<uint8_t> &data)
{
    CapturerData capturerData;
    StateMsg msg(GET_WAKEUP_PCM, nullptr, 0, reinterpret_cast<void *>(&capturerData));
    int32_t ret = ROLE(WakeupEngineImpl).Handle(msg);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("get wakeup pcm failed, ret:%{public}d", ret);
        return -1;
    }

    data.swap(capturerData.data);
    return 0;
}

std::string WakeupEngine::GetEventValue(int32_t uuid)
{
    if (uuid == PROXIMAL_WAKEUP_MODEL_UUID) {
        return "whisper_event";
    }

    return "recognition_event";
}

void WakeupEngine::StartAbility(int32_t uuid)
{
    AAFwk::Want want;
    HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();

    std::string bundleName = historyInfoMgr.GetWakeupEngineBundleName();
    std::string abilityName = historyInfoMgr.GetWakeupEngineAbilityName();
    INTELL_VOICE_LOG_INFO("bundleName:%{public}s, abilityName:%{public}s", bundleName.c_str(), abilityName.c_str());
    want.SetElementName(bundleName, abilityName);
    want.SetParam("serviceName", std::string("intell_voice"));
    want.SetParam("servicePid", getpid());
    want.SetParam("eventType", GetEventValue(uuid));
    AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want);
}

bool WakeupEngine::ResetAdapter()
{
    StateMsg msg(RESET_ADAPTER);
    return (ROLE(WakeupEngineImpl).Handle(msg) == 0 ? true : false);
}

void WakeupEngine::ReleaseAdapter()
{
    StateMsg msg(RELEASE_ADAPTER);
    ROLE(WakeupEngineImpl).Handle(msg);
}
}  // namespace IntellVoice
}  // namespace OHOS
