/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "intell_voice_engine_manager.h"

#include <vector>
#include <fstream>
#include <cstdio>
#include "audio_system_manager.h"
#include "idevmgr_hdi.h"
#include "intell_voice_log.h"
#include "intell_voice_util.h"
#include "engine_factory.h"
#include "wakeup_engine.h"
#include "iservice_registry.h"
#include "intell_voice_generic_factory.h"
#include "memory_guard.h"
#include "iproxy_broker.h"
#include "engine_host_manager.h"
#include "string_util.h"
#include "clone_update_strategy.h"
#include "silence_update_strategy.h"
#include "whisper_update_strategy.h"
#include "update_engine_utils.h"
#include "json/json.h"
#include "intell_voice_sensibility.h"
#include "headset_wakeup_wrapper.h"
#include "engine_callback_message.h"
#include "data_operation_callback.h"
#include "history_info_mgr.h"
#include "intell_voice_definitions.h"

#define LOG_TAG "IntellVoiceEngineManager"

using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;
using OHOS::HDI::DeviceManager::V1_0::IDeviceManager;

namespace OHOS {
namespace IntellVoiceEngine {
static const std::string XIAOYIXIAOYI = "\xE5\xB0\x8F\xE8\x89\xBA\xE5\xB0\x8F\xE8\x89\xBA";
static const std::string LANGUAGE_TYPE_CHN = "zh";
std::atomic<bool> IntellVoiceEngineManager::screenoff_{false};
std::atomic<bool> IntellVoiceEngineManager::g_enrollResult[ENGINE_TYPE_BUT] = {false, false, false};

std::shared_ptr<IntellVoiceEngineManager> IntellVoiceEngineManager::instance_ = nullptr;
std::mutex IntellVoiceEngineManager::instanceMutex_;

IntellVoiceEngineManager::IntellVoiceEngineManager()
{
}

IntellVoiceEngineManager::~IntellVoiceEngineManager()
{
}

sptr<IIntellVoiceEngine> IntellVoiceEngineManager::CreateEngine(IntellVoiceEngineType type,
    const std::string &param, bool reEnroll)
{
    INTELL_VOICE_LOG_INFO("enter, type:%{public}d", type);
    SetEnrollResult(type, false);
    if (ApplyArbitration(type, engines_) != ARBITRATION_OK) {
        INTELL_VOICE_LOG_ERROR("policy manager reject create engine, type:%{public}d", type);
        return nullptr;
    }

    if (type != INTELL_VOICE_WAKEUP) {
        auto engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
        if (engine != nullptr) {
            engine->ReleaseAdapter();
        }
    }

    return CreateEngineInner(type, param, reEnroll);
}

sptr<IIntellVoiceEngine> IntellVoiceEngineManager::CreateEngineInner(IntellVoiceEngineType type,
    const std::string &param, bool reEnroll)
{
    INTELL_VOICE_LOG_INFO("create engine enter, type: %{public}d", type);
    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    sptr<EngineBase> engine = GetEngine(type, engines_);
    if (engine != nullptr) {
        return engine;
    }

    engine = EngineFactory::CreateEngineInst(type, param, reEnroll);
    if (engine == nullptr) {
        INTELL_VOICE_LOG_ERROR("create engine failed, type:%{public}d", type);
        return nullptr;
    }
    engines_[type] = engine;
    INTELL_VOICE_LOG_INFO("create engine ok");
    return engine;
}

int32_t IntellVoiceEngineManager::ReleaseEngineInner(IntellVoiceEngineType type)
{
    OHOS::IntellVoiceUtils::MemoryGuard memoryGuard;
    auto it = engines_.find(type);
    if (it == engines_.end()) {
        INTELL_VOICE_LOG_WARN("there is no engine(%{public}d) in list", type);
        return 0;
    }

    if (it->second != nullptr) {
        it->second->Detach();
        it->second = nullptr;
    }

    engines_.erase(type);
    return 0;
}

void IntellVoiceEngineManager::ClearUserDataInner()
{
    UpdateEngineController::ForceRelease();
    auto engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
    if (engine != nullptr) {
        engine->Detach();
    }
    auto wakeupPhrase = HistoryInfoMgr::GetInstance().GetStringKVPair(KEY_WAKEUP_PHRASE);
    if (!wakeupPhrase.empty()) {
        if (HistoryInfoMgr::GetInstance().GetStringKVPair(KEY_LANGUAGE)
            == LANGUAGE_TYPE_CHN && wakeupPhrase != XIAOYIXIAOYI) {
            wakeupPhrase = "Default";
        }
        EngineHostManager::GetInstance().ClearUserWakeupData(wakeupPhrase);
    }
}

bool IntellVoiceEngineManager::CreateOrResetWakeupEngine()
{
    auto engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
    if (engine != nullptr) {
        INTELL_VOICE_LOG_INFO("wakeup engine is existed");
        engine->ReleaseAdapter();
        if (!engine->ResetAdapter()) {
            INTELL_VOICE_LOG_ERROR("failed to reset adapter");
            return false;
        }
    } else {
        if (CreateEngineInner(INTELL_VOICE_WAKEUP) == nullptr) {
            INTELL_VOICE_LOG_ERROR("failed to create wakeup engine");
            return false;
        }
    }
    return true;
}

int32_t IntellVoiceEngineManager::ServiceStopProc()
{
    sptr<EngineBase> wakeupEngine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
    if (wakeupEngine == nullptr) {
        INTELL_VOICE_LOG_INFO("wakeup engine is not existed");
        return -1;
    }
    wakeupEngine->Detach();
    wakeupEngine->NotifyHeadsetHostEvent(HEADSET_HOST_OFF);
    return 0;
}

bool IntellVoiceEngineManager::RegisterProxyDeathRecipient(IntellVoiceEngineType type,
    const sptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(deathMutex_);
    INTELL_VOICE_LOG_INFO("enter, type:%{public}d", type);
    deathRecipientObj_[type] = object;
    if (type == INTELL_VOICE_ENROLL) {
        proxyDeathRecipient_[type] = new (std::nothrow) IntellVoiceDeathRecipient([&]() {
            INTELL_VOICE_LOG_INFO("receive enroll proxy death recipient, release enroll engine");
            EngineCallbackMessage::CallFunc(HANDLE_RELEASE_ENGINE, INTELL_VOICE_ENROLL);
        });
    } else if (type == INTELL_VOICE_WAKEUP) {
        proxyDeathRecipient_[type] = new (std::nothrow) IntellVoiceDeathRecipient([&]() {
            INTELL_VOICE_LOG_INFO("receive wakeup proxy death recipient, clear wakeup engine callback");
            EngineCallbackMessage::CallFunc(HANDLE_CLEAR_WAKEUP_ENGINE_CB);
        });
    } else if (type == INTELL_VOICE_HEADSET_WAKEUP) {
        proxyDeathRecipient_[type] = new (std::nothrow) IntellVoiceDeathRecipient([&]() {
            INTELL_VOICE_LOG_INFO("receive headset wakeup proxy death recipient, notify headset host off");
            EngineCallbackMessage::CallFunc(HANDLE_HEADSET_HOST_DIE);
        });
    } else {
        INTELL_VOICE_LOG_ERROR("invalid type:%{public}d", type);
        return false;
    }

    if (proxyDeathRecipient_[type] == nullptr) {
        INTELL_VOICE_LOG_ERROR("create death recipient failed");
        return false;
    }

    return deathRecipientObj_[type]->AddDeathRecipient(proxyDeathRecipient_[type]);
}

bool IntellVoiceEngineManager::DeregisterProxyDeathRecipient(IntellVoiceEngineType type)
{
    std::lock_guard<std::mutex> lock(deathMutex_);
    INTELL_VOICE_LOG_INFO("enter, type:%{public}d", type);
    if (deathRecipientObj_.count(type) == 0 || deathRecipientObj_[type] == nullptr) {
        INTELL_VOICE_LOG_ERROR("death obj is nullptr, type:%{public}d", type);
        return false;
    }
    if (proxyDeathRecipient_.count(type) == 0 || proxyDeathRecipient_[type] == nullptr) {
        INTELL_VOICE_LOG_ERROR("death recipient is nullptr, type:%{public}d", type);
        deathRecipientObj_.erase(type);
        return false;
    }

    auto ret = deathRecipientObj_[type]->RemoveDeathRecipient(proxyDeathRecipient_[type]);
    deathRecipientObj_.erase(type);
    proxyDeathRecipient_.erase(type);
    return ret;
}

bool IntellVoiceEngineManager::AnyEngineExist(const std::vector<IntellVoiceEngineType> &types)
{
    for (const auto &type : types) {
        if (IsEngineExist(type)) {
            return true;
        }
    }
    return false;
}

bool IntellVoiceEngineManager::IsEngineExist(IntellVoiceEngineType type)
{
    sptr<EngineBase> engine = GetEngine(type, engines_);
    if (engine != nullptr) {
        INTELL_VOICE_LOG_INFO("engine exist, type:%{public}d", type);
        return true;
    }

    if (type == INTELL_VOICE_UPDATE && GetUpdateState()) {
        INTELL_VOICE_LOG_ERROR("update is running");
        return true;
    }

    return false;
}

bool IntellVoiceEngineManager::CreateUpdateEngine(const std::string &param, bool reEnroll)
{
    sptr<IIntellVoiceEngine> updateEngine = CreateEngine(INTELL_VOICE_UPDATE, param, reEnroll);
    if (updateEngine == nullptr) {
        INTELL_VOICE_LOG_ERROR("updateEngine is nullptr");
        return false;
    }

    return true;
}

void IntellVoiceEngineManager::ReleaseUpdateEngine()
{
    EngineCallbackMessage::CallFunc(RELEASE_ENGINE, INTELL_VOICE_UPDATE);
}

int32_t IntellVoiceEngineManager::GetUploadFiles(int numMax, std::vector<UploadFilesFromHdi> &files)
{
    std::vector<UploadHdiFile> hdiFiles;
    auto ret = EngineHostManager::GetInstance().GetUploadFiles(numMax, hdiFiles);
    if (ret != 0) {
        INTELL_VOICE_LOG_INFO("failed to get upload file, ret:%{public}d", ret);
        return ret;
    }

    for (auto item : hdiFiles) {
        UploadFilesFromHdi file;
        file.type = item.type;
        file.filesDescription = item.filesDescription;
        for (auto mem : item.filesContent) {
            file.filesContent.emplace_back(mem);
        }
        file.type = item.type;
        files.emplace_back(file);
    }
    return ret;
}

std::string IntellVoiceEngineManager::GetParameter(const std::string &key)
{
    std::string val = "";

    if (key == "isEnrolled") {
        HistoryInfoMgr &historyInfoMgr = HistoryInfoMgr::GetInstance();
        val = historyInfoMgr.GetStringKVPair(KEY_WAKEUP_VESRION).empty() ? "false" : "true";
        INTELL_VOICE_LOG_INFO("get is enroll result %{public}s", val.c_str());
    } else if (key == "isNeedReEnroll") {
        val = UpdateEngineUtils::IsVersionUpdate() ? "true" : "false";
        INTELL_VOICE_LOG_INFO("get nedd reenroll result %{public}s", val.c_str());
    } else if (key == "isWhispering") {
        auto audioSystemManager = AudioStandard::AudioSystemManager::GetInstance();
        if (audioSystemManager == nullptr) {
            INTELL_VOICE_LOG_ERROR("audioSystemManager is nullptr");
            return val;
        }
        val = std::to_string(audioSystemManager->IsWhispering());
        INTELL_VOICE_LOG_INFO("get isWhispering result %{public}s", val.c_str());
    }

    return val;
}

int32_t IntellVoiceEngineManager::SetParameter(const std::string &sensibility)
{
    auto engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
    if (engine != nullptr) {
        engine->SetParameter(SENSIBILITY_TEXT + sensibility);
    }
    return 0;
}

int32_t IntellVoiceEngineManager::GetWakeupSourceFilesList(std::vector<std::string> &cloneFiles)
{
    return EngineHostManager::GetInstance().GetWakeupSourceFilesList(cloneFiles);
}

int32_t IntellVoiceEngineManager::GetWakeupSourceFile(const std::string &filePath, std::vector<uint8_t> &buffer)
{
    return EngineHostManager::GetInstance().GetWakeupSourceFile(filePath, buffer);
}

int32_t IntellVoiceEngineManager::SendWakeupFile(const std::string &filePath, const std::vector<uint8_t> &buffer)
{
    if (buffer.data() == nullptr) {
        INTELL_VOICE_LOG_ERROR("send update callback is nullptr");
    }

    return EngineHostManager::GetInstance().SendWakeupFile(filePath, buffer);
}

int32_t IntellVoiceEngineManager::CloneUpdate(const std::string &wakeupInfo, const sptr<IRemoteObject> &object)
{
    sptr<IIntelligentVoiceUpdateCallback> updateCallback = iface_cast<IIntelligentVoiceUpdateCallback>(object);
    if (updateCallback == nullptr) {
        INTELL_VOICE_LOG_ERROR("update callback is nullptr");
        return -1;
    }

    if (wakeupInfo.empty()) {
        INTELL_VOICE_LOG_ERROR("clone info empty");
        return -1;
    }

    std::shared_ptr<CloneUpdateStrategy> cloneStrategy =
        std::make_shared<CloneUpdateStrategy>(wakeupInfo, updateCallback);
    if (cloneStrategy == nullptr) {
        INTELL_VOICE_LOG_ERROR("clone strategy is nullptr");
        return -1;
    }

    INTELL_VOICE_LOG_INFO("enter");
    std::shared_ptr<IUpdateStrategy> strategy = std::dynamic_pointer_cast<IUpdateStrategy>(cloneStrategy);
    return CreateUpdateEngineUntilTime(strategy);
}

int32_t IntellVoiceEngineManager::SilenceUpdate()
{
    std::shared_ptr<SilenceUpdateStrategy> silenceStrategy = std::make_shared<SilenceUpdateStrategy>("");
    if (silenceStrategy == nullptr) {
        INTELL_VOICE_LOG_ERROR("silence strategy is nullptr");
        return -1;
    }

    INTELL_VOICE_LOG_INFO("enter");
    std::shared_ptr<IUpdateStrategy> strategy = std::dynamic_pointer_cast<IUpdateStrategy>(silenceStrategy);
    return CreateUpdateEngineUntilTime(strategy);
}

int32_t IntellVoiceEngineManager::WhisperVprUpdate(bool reEnroll)
{
    INTELL_VOICE_LOG_INFO("enter");
    std::shared_ptr<IUpdateStrategy> strategy = std::make_shared<WhisperUpdateStrategy>("WhisperVprUpdate");
    if (strategy == nullptr) {
        INTELL_VOICE_LOG_ERROR("strategy is nullptr");
        return -1;
    }

    return CreateUpdateEngineUntilTime(strategy, reEnroll);
}


std::string IntellVoiceEngineManager::GetDspSensibility(const std::string &sensibility,
    const std::string &dspFeature, const std::string &configPath)
{
    return IntellVoiceSensibility::GetDspSensibility(sensibility, dspFeature, configPath);
}

void IntellVoiceEngineManager::HeadsetHostDie()
{
    auto engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
    if (engine != nullptr) {
        engine->NotifyHeadsetHostEvent(HEADSET_HOST_OFF);
    }
}

bool IntellVoiceEngineManager::IsNeedUpdateComplete(int32_t result, const std::string &param)
{
    bool isLast = false;
    UpdateEngineController::UpdateCompleteProc(static_cast<UpdateState>(result), param, isLast);
    if ((IsEngineExist(INTELL_VOICE_ENROLL)) || (!isLast)) {
        INTELL_VOICE_LOG_INFO("enroll engine is existed, or is not last:%{public}d", isLast);
        return false;
    }
    return true;
}

bool IntellVoiceEngineManager::IsNeedUpdateRetry()
{
    if (UpdateEngineController::UpdateRetryProc()) {
        INTELL_VOICE_LOG_INFO("retry to update or already force to stop");
        return false;
    }
    if (IsEngineExist(INTELL_VOICE_ENROLL)) {
        INTELL_VOICE_LOG_INFO("enroll engine is existed, do nothing");
        return false;
    }
    return true;
}

void IntellVoiceEngineManager::EngineOnDetected(int32_t uuid)
{
    auto engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
    engine->OnDetected(uuid);
}

void IntellVoiceEngineManager::ClearWakeupEngineCb()
{
    sptr<EngineBase> engine = GetEngine(INTELL_VOICE_WAKEUP, engines_);
    if (engine != nullptr) {
        INTELL_VOICE_LOG_INFO("clear wakeup engine callback");
        engine->SetCallback(nullptr);
    }
}

bool IntellVoiceEngineManager::GetScreenOff()
{
    return screenoff_.load();
}

void IntellVoiceEngineManager::SetScreenOff(bool value)
{
    screenoff_.store(value);
}

void IntellVoiceEngineManager::SetDspSensibility(const std::string &sensibility)
{
    auto ret = EngineCallbackMessage::CallFunc(TRIGGERMGR_GET_PARAMETER, KEY_GET_WAKEUP_FEATURE);
    std::string features = "";
    if (ret.has_value()) {
        try {
            features = std::any_cast<std::string>(*ret);
        } catch (const std::bad_any_cast&) {
            INTELL_VOICE_LOG_ERROR("msg bus bad any cast");
            return;
        }
    } else {
        INTELL_VOICE_LOG_ERROR("msg bus return no value");
        return;
    }
    auto value = GetDspSensibility(sensibility, features, WAKEUP_CONFIG_PATH);
    if (value.empty()) {
        INTELL_VOICE_LOG_ERROR("no sensibility value");
        return;
    }
    EngineCallbackMessage::CallFunc(TRIGGERMGR_SET_PARAMETER, "WAKEUP_SENSIBILITY", value);
}

void IntellVoiceEngineManager::OnServiceStart()
{
    LoadIntellVoiceHost();
}

void IntellVoiceEngineManager::OnServiceStop()
{
    UnloadIntellVoiceHost();
}

void IntellVoiceEngineManager::LoadIntellVoiceHost()
{
    auto devmgr = IDeviceManager::Get();
    if (devmgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("Get devmgr failed");
        return;
    }
    INTELL_VOICE_LOG_INFO("Get devmgr success");
    devmgr->UnloadDevice("intell_voice_engine_manager_service");
    devmgr->LoadDevice("intell_voice_engine_manager_service");

    if (!EngineHostManager::GetInstance().Init()) {
        INTELL_VOICE_LOG_ERROR("init engine host failed");
        return;
    }

    EngineHostManager::GetInstance().RegisterEngineHDIDeathRecipient();
    EngineHostManager::GetInstance().SetDataOprCallback();
}

void IntellVoiceEngineManager::UnloadIntellVoiceHost()
{
    auto devmgr = IDeviceManager::Get();
    if (devmgr != nullptr) {
        INTELL_VOICE_LOG_INFO("Get devmgr success");
        EngineHostManager::GetInstance().DeregisterEngineHDIDeathRecipient();
        devmgr->UnloadDevice("intell_voice_engine_manager_service");
    } else {
        INTELL_VOICE_LOG_ERROR("Get devmgr failed");
    }
}
}  // namespace IntellVoiceEngine
}  // namespace OHOS
