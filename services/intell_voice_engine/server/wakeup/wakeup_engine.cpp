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
#include "idevmgr_hdi.h"
#include "intell_voice_service_manager.h"
#include "intell_voice_log.h"
#include "headset_host_manager.h"

#define LOG_TAG "WakeupEngine"

using namespace OHOS::IntellVoiceUtils;
using OHOS::HDI::DeviceManager::V1_0::IDeviceManager;

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
    {
        std::lock_guard<std::mutex> lock(headsetMutex_);
        if ((headsetImpl_ != nullptr) && (headsetImpl_->GetHeadsetAwakeState() == 1)) {
            INTELL_VOICE_LOG_INFO("headset wakeup is exist");
            return;
        }
    }

    std::thread([uuid]() { WakeupEngine::StartAbility(GetEventValue(uuid)); }).detach();
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
    {
        std::lock_guard<std::mutex> lock(headsetMutex_);
        if (headsetImpl_ != nullptr) {
            headsetImpl_->Handle(msg);
        }
    }
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

int32_t WakeupEngine::NotifyHeadsetWakeEvent()
{
    INTELL_VOICE_LOG_INFO("enter");
    std::lock_guard<std::mutex> lock(headsetMutex_);
    if (headsetImpl_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("headset impl is nullptr");
        return -1;
    }

    std::thread([]() { WakeupEngine::StartAbility("headset_event"); }).detach();

    StateMsg msg(START_RECOGNIZE);
    return headsetImpl_->Handle(msg);
}

int32_t WakeupEngine::HandleHeadsetOff()
{
    {
        std::lock_guard<std::mutex> lock(headsetMutex_);
        if (headsetImpl_ != nullptr) {
            StateMsg msg(RELEASE);
            if (headsetImpl_->Handle(msg) != 0) {
                INTELL_VOICE_LOG_ERROR("release headset wakeup engine impl failed");
            }
            headsetImpl_ = nullptr;
        }
    }
    HeadsetHostManager::GetInstance().DeregisterEngineHDIDeathRecipient();
    auto devmgr = IDeviceManager::Get();
    if (devmgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("get devmgr failed");
        return -1;
    }
    devmgr->UnloadDevice("intell_voice_headset_manager_service");
    return 0;
}

int32_t WakeupEngine::HandleHeadsetOn()
{
    auto devmgr = IDeviceManager::Get();
    if (devmgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("get devmgr failed");
        return -1;
    }
    devmgr->LoadDevice("intell_voice_headset_manager_service");
    if (!HeadsetHostManager::GetInstance().Init()) {
        INTELL_VOICE_LOG_ERROR("init headset host failed");
        return -1;
    }

    std::lock_guard<std::mutex> lock(headsetMutex_);
    headsetImpl_ = UniquePtrFactory<HeadsetWakeupEngineImpl>::CreateInstance();
    if (headsetImpl_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to allocate headset impl");
        return -1;
    }

    StateMsg msg(INIT);
    if (headsetImpl_->Handle(msg) != 0) {
        INTELL_VOICE_LOG_ERROR("init headset wakeup engine impl failed");
        return -1;
    }
    return 0;
}

int32_t WakeupEngine::NotifyHeadsetHostEvent(HeadsetHostEventType event)
{
    INTELL_VOICE_LOG_INFO("enter, event:%{public}d", event);
    if (event == HEADSET_HOST_OFF) {
        return HandleHeadsetOff();
    } else if (event == HEADSET_HOST_ON) {
        return HandleHeadsetOn();
    }

    INTELL_VOICE_LOG_WARN("invalid event:%{public}d", event);
    return 0;
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

void WakeupEngine::StartAbility(const std::string &event)
{
    AAFwk::Want want;
    HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();

    std::string bundleName = historyInfoMgr.GetWakeupEngineBundleName();
    std::string abilityName = historyInfoMgr.GetWakeupEngineAbilityName();
    INTELL_VOICE_LOG_INFO("bundleName:%{public}s, abilityName:%{public}s", bundleName.c_str(), abilityName.c_str());
    want.SetElementName(bundleName, abilityName);
    want.SetParam("serviceName", std::string("intell_voice"));
    want.SetParam("servicePid", getpid());
    want.SetParam("eventType", event);
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
