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

#include "string_util.h"
#include <fstream>
#include <iterator>
#include <ctime>
#include <sys/time.h>
#include "intell_voice_log.h"

#undef LOG_TAG
#define LOG_TAG "StringUtil"

using namespace std;

namespace OHOS {
namespace IntellVoiceUtils {
static const char16_t DELIMITER_EQUAL_SIGN = '=';

// #ABC#WWW##XYZ#---->vector[0]: ABC, vector[1]: WWW, vector[2]: XYZ
void StringUtil::Split(const string &str, const string &sep, vector<string> &res)
{
    res.clear();
    string tmpStr = str;
    unsigned int startPos = 0;
    size_t findPos = tmpStr.find(sep, startPos);
    while (findPos != string::npos) {
        if (findPos > startPos) {
            res.push_back(tmpStr.substr(static_cast<uint32_t>(startPos), static_cast<uint32_t>((findPos - startPos))));
        }
        startPos = findPos + sep.length();
        findPos = tmpStr.find(sep, startPos);
    }

    if (startPos < tmpStr.length()) {
        res.push_back(tmpStr.substr(startPos));
    }
}

// the func is called for AcousticPruner and AcousticPrunerExtra file
std::vector<string> StringUtil::StringSplit(string &str, const string &pattern)
{
    string::size_type pos;
    vector<string> result;
    str += pattern;
    unsigned int size = str.size();

    for (unsigned int i = 0; i < size; i++) {
        pos = str.find(pattern, i);
        if (pos < size) {
            string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }

    return result;
}

template <typename T> string StringUtil::PrintVector(vector<T> &array, string &delimiter)
{
    if (array.empty()) {
        return "";
    }

    ostringstream oss;
    copy(array.begin(), array.end() - 1, ostream_iterator<T>(oss, delimiter.c_str()));
    oss << array.back();

    return oss.str();
}

void StringUtil::TrimSpecialChars(string &str)
{
    size_t sp = 0;

    while (sp < str.size()) {
        size_t ep = str.find_first_of(" \t\n\v\f\r,.:;!~@#$%^&*()`?/-+", sp);
        if (ep != string::npos) {
            str.erase(ep, 1);
            sp = ep;
        } else {
            break;
        }
    }
}

uint32_t StringUtil::CalSubStrNum(const string &str, const string &subStr)
{
    if (subStr.size() == 0) {
        return 0;
    }
    uint32_t count = 0;
    string::size_type pos = 0;
    pos = str.find(subStr, pos);
    while (pos != string::npos) {
        count++;
        pos = pos + subStr.size();
        pos = str.find(subStr, pos);
    }

    return count;
}
bool StringUtil::SplitLineToPair(const std::string &line, std::string &first, std::string &second)
{
    if (line.empty()) {
        INTELL_VOICE_LOG_ERROR("line is empty");
        return false;
    }
    // pinyin:words
    size_t pos = line.find(DELIMITER_EQUAL_SIGN);
    // not find delimiter or it is the last char.
    if (string::npos == pos || (line.length() - 1) == pos) {
        return false;
    }

    first = line.substr(0, pos);
    second = line.substr(pos + 1, string::npos);
    // trim left and right spaces.
    Trim(first);
    Trim(second);

    if (first.empty() || second.empty()) {
        INTELL_VOICE_LOG_ERROR("line is invalid, first:%{public}s, second:%{public}s", first.c_str(), second.c_str());
        return false;
    }
    return true;
}
}
}

