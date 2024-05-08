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
#include "engine_util.h"
#include <ashmem.h>

#include "intell_voice_log.h"
#include "string_util.h"
#include "scope_guard.h"
#include "engine_host_manager.h"
#include "trigger_manager.h"
#include "intell_voice_service_manager.h"

#define LOG_TAG "EngineUtils"

using namespace OHOS::IntellVoiceUtils;
using namespace OHOS::HDI::IntelligentVoice::Engine::V1_0;
using namespace OHOS::IntellVoiceTrigger;

namespace OHOS {
namespace IntellVoiceEngine {
static const std::string KEY_GET_WAKEUP_FEATURE = "wakeup_features";
static const std::string LANGUAGE_TEXT = "language=";
static const std::string AREA_TEXT = "area=";

EngineUtil::EngineUtil()
{
    desc_.adapterType = ADAPTER_TYPE_BUT;
}

int32_t EngineUtil::SetParameter(const std::string &keyValueList)
{
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }
    return adapter_->SetParameter(keyValueList);
}

std::string EngineUtil::GetParameter(const std::string &key)
{
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return "";
    }

    std::string value;
    adapter_->GetParameter(key, value);
    return value;
}

int32_t EngineUtil::WriteAudio(const uint8_t *buffer, uint32_t size)
{
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }

    if (buffer == nullptr || size == 0) {
        INTELL_VOICE_LOG_ERROR("buffer is invalid, size:%{public}u", size);
        return -1;
    }

    std::vector<uint8_t> audioBuff(&buffer[0], &buffer[size]);
    return adapter_->WriteAudio(audioBuff);
}

int32_t EngineUtil::Stop()
{
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }
    return adapter_->Stop();
}

int32_t EngineUtil::GetWakeupPcm(std::vector<uint8_t> &data)
{
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }
    return adapter_->GetWakeupPcm(data);
}

int32_t EngineUtil::Evaluate(const std::string &word, EvaluationResultInfo &info)
{
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return -1;
    }

    return adapter_->Evaluate(word, info);
}

bool EngineUtil::SetDspFeatures()
{
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return false;
    }

    auto triggerMgr = TriggerManager::GetInstance();
    if (triggerMgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger manager is nullptr");
        return false;
    }

    std::string features = triggerMgr->GetParameter(KEY_GET_WAKEUP_FEATURE);
    if (features == "") {
        INTELL_VOICE_LOG_WARN("failed to get wakeup dsp feature");
        features = HistoryInfoMgr::GetInstance().GetWakeupDspFeature();
        if (features == "") {
            INTELL_VOICE_LOG_WARN("no historical wakeup dsp feature");
            return false;
        }
    } else {
        HistoryInfoMgr::GetInstance().SetWakeupDspFeature(features);
    }

    std::string kvPair = KEY_GET_WAKEUP_FEATURE + "=" + features;
    adapter_->SetParameter(kvPair);
    return true;
}

void EngineUtil::SplitStringToKVPair(const std::string &inputStr, std::map<std::string, std::string> &kvpairs)
{
    std::vector<std::string> paramsList;
    StringUtil::Split(inputStr, ";", paramsList);
    for (auto &it : paramsList) {
        std::string key;
        std::string value;
        if (StringUtil::SplitLineToPair(it, key, value)) {
            kvpairs[key] = value;
            INTELL_VOICE_LOG_INFO("key:%{public}s, value:%{public}s", key.c_str(), value.c_str());
        }
    }
}

void EngineUtil::WriteBufferFromAshmem(uint8_t *&buffer, uint32_t size, sptr<OHOS::Ashmem> ashmem)
{
    if (!ashmem->MapReadOnlyAshmem()) {
        INTELL_VOICE_LOG_ERROR("map ashmem failed");
        return;
    }

    const uint8_t *tmpBuffer = static_cast<const uint8_t *>(ashmem->ReadFromAshmem(size, 0));
    if (tmpBuffer == nullptr) {
        INTELL_VOICE_LOG_ERROR("read from ashmem failed");
        return;
    }

    buffer = new (std::nothrow) uint8_t[size];
    if (buffer == nullptr) {
        INTELL_VOICE_LOG_ERROR("allocate buffer failed");
        return;
    }

    if (memcpy_s(buffer, size, tmpBuffer, size) != 0) {
        INTELL_VOICE_LOG_ERROR("memcpy_s failed");
        return;
    }
}

void EngineUtil::ProcDspModel(OHOS::HDI::IntelligentVoice::Engine::V1_0::ContentType type)
{
    INTELL_VOICE_LOG_INFO("enter");
    uint8_t *buffer = nullptr;
    uint32_t size = 0;
    sptr<Ashmem> ashmem;
    adapter_->Read(type, ashmem);
    if (ashmem == nullptr) {
        INTELL_VOICE_LOG_ERROR("ashmem is nullptr");
        return;
    }

    ON_SCOPE_EXIT_WITH_NAME(ashmemExit)
    {
        INTELL_VOICE_LOG_DEBUG("close ashmem");
        ashmem->UnmapAshmem();
        ashmem->CloseAshmem();
    };

    size = static_cast<uint32_t>(ashmem->GetAshmemSize());
    if (size == 0) {
        INTELL_VOICE_LOG_ERROR("size is zero");
        return;
    }

    WriteBufferFromAshmem(buffer, size, ashmem);
    if (buffer == nullptr) {
        INTELL_VOICE_LOG_ERROR("buffer is nullptr");
        return;
    }

    ON_SCOPE_EXIT_WITH_NAME(bufferExit)
    {
        INTELL_VOICE_LOG_DEBUG("now delete buffer");
        delete[] buffer;
        buffer = nullptr;
    };

    std::shared_ptr<GenericTriggerModel> model = std::make_shared<GenericTriggerModel>(
        VOICE_WAKEUP_MODEL_UUID, 1, TriggerModel::TriggerModelType::VOICE_WAKEUP_TYPE);
    if (model == nullptr) {
        INTELL_VOICE_LOG_ERROR("model is null");
        return;
    }
    model->SetData(buffer, size);
    auto triggerMgr = TriggerManager::GetInstance();
    if (triggerMgr == nullptr) {
        INTELL_VOICE_LOG_ERROR("trigger manager is nullptr");
        return;
    }
    triggerMgr->UpdateModel(model);
}

void EngineUtil::SetLanguage()
{
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return;
    }

    std::string language = HistoryInfoMgr::GetInstance().GetLanguage();
    if (language.empty()) {
        INTELL_VOICE_LOG_WARN("language is empty");
        return;
    }
    adapter_->SetParameter(LANGUAGE_TEXT + language);
}

void EngineUtil::SetArea()
{
    if (adapter_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("adapter is nullptr");
        return;
    }

    std::string area = HistoryInfoMgr::GetInstance().GetArea();
    if (area.empty()) {
        INTELL_VOICE_LOG_WARN("area is empty");
        return;
    }

    adapter_->SetParameter(AREA_TEXT + area);
}
}
}
