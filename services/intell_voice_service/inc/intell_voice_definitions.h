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
#ifndef INTELL_VOICE_DEFINITIONS_H
#define INTELL_VOICE_DEFINITIONS_H
#include <string>
namespace OHOS {
namespace IntellVoiceEngine {
constexpr int32_t VOICE_WAKEUP_MODEL_UUID = 1;
constexpr int32_t PROXIMAL_WAKEUP_MODEL_UUID = 2;
const std::string WAKEUP_KEY = "intell_voice_trigger_enabled";
const std::string WHISPER_KEY = "intell_voice_trigger_whisper";
const std::string IMPROVE_KEY = "intell_voice_improve_enabled";
const std::string SHORTWORD_KEY = "intell_voice_trigger_shortword";
const std::string SENSIBILITY_TEXT = "sensibility=";

const std::string KEY_GET_WAKEUP_FEATURE = "wakeup_features";
const std::string KEY_WAKEUP_ENGINE_BUNDLE_NAME = "WakeupEngineBundleName";
const std::string KEY_WAKEUP_ENGINE_ABILITY_NAME = "WakeupEngineAbilityName";
const std::string KEY_WAKEUP_VESRION = "WakeupVersion";
const std::string KEY_LANGUAGE = "Language";
const std::string KEY_AREA = "Area";
const std::string KEY_WAKEUP_PHRASE = "WakeupPhrase";
const std::string KEY_ENROLL_ENGINE_UID = "EnrollEngineUid";
const std::string KEY_SENSIBILITY = "Sensibility";
const std::string KEY_WAKEUP_DSP_FEATURE = "WakeupDspFeature";

static const std::string WHISPER_MODEL_PATH =
    "/sys_prod/variant/region_comm/china/etc/intellvoice/wakeup/dsp/whisper_wakeup_dsp_config";
static const std::string VAD_MODEL_PATH =
    "/sys_prod/variant/region_comm/china/etc/intellvoice/wakeup/ap/kws2_acousticsModel6.pb";
static const std::string WAKEUP_CONFIG_USER_PATH =
    "/sys_prod/variant/region_comm/china/etc/intellvoice/wakeup/ap/wakeup_config_user.json";
static const std::string WAKEUP_CONFIG_PATH =
    "/sys_prod/variant/region_comm/china/etc/intellvoice/wakeup/ap/wakeup_config.json";
static const std::string SINGLE_LEVEL_MODEL_PATH =
    "/sys_prod/variant/region_comm/china/etc/intellvoice/wakeup/dsp/wakeup_dsp_config";
}  // namespace IntellVoice
namespace IntellVoiceTrigger {
    enum TriggerModelType {
        UNKNOWN_TYPE = -1,
        GENERIC_TYPE = 1,
        VOICE_WAKEUP_TYPE = 1,
        PROXIMAL_WAKEUP_TYPE = 2,
    };
} // namespace IntellVoice
}  // namespace OHOS
#endif