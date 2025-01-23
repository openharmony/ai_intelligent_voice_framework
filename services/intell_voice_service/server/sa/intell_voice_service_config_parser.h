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

#ifndef INTELL_VOICE_SERVICE_CONFIG_PARSER_H
#define INTELL_VOICE_SERVICE_CONFIG_PARSER_H

#include <unordered_map>
#include <string>
#include <sstream>
#include <libxml/parser.h>
#include <libxml/tree.h>

namespace OHOS {
namespace IntellVoiceEngine {
enum WakeupLevelOption {
    WAKEUP_LEVEL_DEFULT,
    WAKEUP_LEVEL_SINGLE,
};

class AudioWakeupConfigParser {
public:
    explicit AudioWakeupConfigParser();
    ~AudioWakeupConfigParser();
    int32_t GetWakeupLevel();

private:
    bool LoadConfig();
    void Destroy();
    bool Parse();
    bool ParseInternal(xmlNode *node);
    void ParserAttributeInfo(xmlNode *node);
    void ParserOption(const std::string &propName);
    void SetWakeupLevel(int32_t wakeupLevel);
    std::string ExtractPropertyValue(const std::string &propName, xmlNode *node);

    xmlDoc *doc_ = nullptr;
    int32_t wakeupLevel_ = WAKEUP_LEVEL_DEFULT;
};
} // namespace IntellVoiceEngine
} // namespace OHOS

#endif // AUDIO_WAKEUP_CONFIG_PARSER_H
