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

#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <string>
#include <vector>
#include <sstream>
#include "nocopyable.h"
#include "securec.h"

namespace OHOS {
namespace IntellVoiceUtils {
class StringUtil {
public:
    StringUtil() {}
    ~StringUtil() {}
    static void Split(const std::string &str, const std::string &sep, std::vector<std::string> &res);
    static std::vector<std::string> StringSplit(std::string &str, const std::string &pattern);
    static void TrimL(std::string &str);
    static void TrimR(std::string &str);
    static void Trim(std::string &str);
    template <typename T> static std::string PrintVector(std::vector<T> &array, std::string &delimiter);
    static std::string Float2String(float arg, int32_t precision = 6);
    static std::string Int2String(int32_t arg);
    static void StringToLower(std::string &str);
    static std::string StringToUpper(const std::string &str);
    static std::string StringToUpperX(const std::string &str);
    static void TrimSpecialChars(std::string &str);
    static uint32_t CalSubStrNum(const std::string &str, const std::string &subStr);
    static bool SplitLineToPair(const std::string &line, std::string &first, std::string &second);

private:
    DISALLOW_COPY_AND_MOVE(StringUtil);
};

// remove space of left side, ascii
inline void StringUtil::TrimL(std::string &str)
{
    size_t p = str.find_first_not_of(" \t\n\v\f\r");
    if (p == std::string::npos) {
        str.clear();
    } else {
        str = str.substr(p);
    }
}

// remove space of right side, ascii
inline void StringUtil::TrimR(std::string &str)
{
    size_t p = str.find_last_not_of(" \t\n\v\f\r");
    if (p == std::string::npos) {
        str.clear();
    } else {
        str.erase(p + 1);
    }
}

// remove space of both side, ascii
inline void StringUtil::Trim(std::string &str)
{
    TrimL(str);
    TrimR(str);
}

inline std::string StringUtil::Float2String(float arg, int32_t precision)
{
    // warning -- local variable 'score' declared not subsequently referenced is false alarm, ignore
    char score[100] = {'\0'}; //lint !e529
    if (sprintf_s(score, sizeof(score), "%.*f", precision, arg) == -1) {
        return "";
    }

    return std::string(score);
}

inline std::string StringUtil::Int2String(int32_t arg)
{
    static const int32_t RPESICION = 9;
    std::stringstream ss;

    ss.precision(RPESICION);
    ss << arg;

    return ss.str();
}

inline void StringUtil::StringToLower(std::string &str)
{
    for (uint32_t k = 0; k < str.size(); k++) {
        str[k] = static_cast<char>(tolower(str[k]));
    }
}

inline std::string StringUtil::StringToUpper(const std::string &str)
{
    std::string upstr = str;
    for (uint32_t k = 0; k < str.size(); k++) {
        upstr[k] = static_cast<char>(toupper(str[k]));
    }

    return upstr;
}

inline std::string StringUtil::StringToUpperX(const std::string &str)
{
    std::string upstr = str;
    for (uint32_t i = 0; i < upstr.size(); i++) {
        if (upstr[i] <= 'z' && upstr[i] >= 'a') {
            upstr[i] -= 32; // 32 is the difference between lowercase and uppercase ASCII
        }
    }

    return upstr;
}
}
}
#endif
