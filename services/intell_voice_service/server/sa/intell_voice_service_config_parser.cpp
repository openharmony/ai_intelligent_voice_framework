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

#include "intell_voice_service_config_parser.h"
#include "intell_voice_log.h"
#include "string_util.h"

#ifndef LOG_TAG
#define LOG_TAG "AudioWakeupConfigParser"
#endif

using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {
static constexpr char CONFIG_FILE[] = "/vendor/etc/audio/audio_wakeup_config.xml";
static constexpr char CHIP_PROD_ONFIG_FILE[] = "/chip_prod/etc/audio/audio_wakeup_config.xml";

AudioWakeupConfigParser::AudioWakeupConfigParser()
{
    LoadConfig();
}

AudioWakeupConfigParser::~AudioWakeupConfigParser()
{
    Destroy();
}

bool AudioWakeupConfigParser::LoadConfig()
{
    doc_ = xmlReadFile(CONFIG_FILE, nullptr, 0);
    if (doc_ == nullptr) {
        doc_ = xmlReadFile(CHIP_PROD_ONFIG_FILE, nullptr, 0);
        CHECK_CONDITION_RETURN_FALSE(doc_ == nullptr, "Not found audio_wakeup_config.xml!");
    }

    if (!Parse()) {
        return false;
    }

    return true;
}

bool AudioWakeupConfigParser::Parse()
{
    xmlNode *root = xmlDocGetRootElement(doc_);
    CHECK_CONDITION_RETURN_FALSE(root == nullptr, "xmlDocGetRootElement failed");
    if (xmlStrcmp(root->name, reinterpret_cast<const xmlChar*>("audioWakeupConfiguration"))) {
        INTELL_VOICE_LOG_ERROR("Missing tag - audioWakeupConfiguration");
        return false;
    }

    bool ret = ParseInternal(root);
    CHECK_CONDITION_RETURN_FALSE(!ret, "audio wakeup config parser failed");
    return true;
}

bool AudioWakeupConfigParser::ParseInternal(xmlNode *node)
{
    for (xmlNode *currNode = node; currNode != NULL; currNode = currNode->next) {
        if (XML_ELEMENT_NODE == currNode->type) {
            if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("attribute"))) {
                ParserAttributeInfo(currNode);
            } else {
                ParseInternal((currNode->xmlChildrenNode));
            }
        }
    }
    return true;
}

void AudioWakeupConfigParser::ParserAttributeInfo(xmlNode *node)
{
    xmlNode *strategyNode = node;
    std::string name = ExtractPropertyValue("name", strategyNode);
    std::string option = ExtractPropertyValue("option", strategyNode);

    if (name == "wakeup_level") {
        ParserOption(option);
    }
}

void AudioWakeupConfigParser::ParserOption(const std::string &propName)
{
    std::vector<std::string> buf;
    StringUtil::Split(propName, ",", buf);
    for (const auto &name : buf) {
        if (name == "single_level") {
            SetWakeupLevel(WAKEUP_LEVEL_SINGLE);
        }
    }
}

void AudioWakeupConfigParser::SetWakeupLevel(int32_t wakeupLevel)
{
    wakeupLevel_ = wakeupLevel;
}

int32_t AudioWakeupConfigParser::GetWakeupLevel()
{
    return wakeupLevel_;
}

std::string AudioWakeupConfigParser::ExtractPropertyValue(const std::string &propName, xmlNode *node)
{
    std::string propValue = "";
    xmlChar *tempValue = nullptr;

    if (xmlHasProp(node, reinterpret_cast<const xmlChar*>(propName.c_str()))) {
        tempValue = xmlGetProp(node, reinterpret_cast<const xmlChar*>(propName.c_str()));
    }

    if (tempValue != nullptr) {
        propValue = reinterpret_cast<const char*>(tempValue);
        xmlFree(tempValue);
    }

    return propValue;
}

void AudioWakeupConfigParser::Destroy()
{
    if (doc_ != nullptr) {
        xmlFreeDoc(doc_);
    }
}
} // namespace IntellVoiceEngine
} // namespace OHOS
