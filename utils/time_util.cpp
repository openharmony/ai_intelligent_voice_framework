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
#include "time_util.h"
#include <string>
#include <stdexcept>
#include <sstream>
#include <ctime>
#include <sys/time.h>
#include <chrono>
#include "intell_voice_log.h"

#define LOG_TAG "TimeUtil"

using namespace std;
static const int INVALID_TIME_T = -1;

namespace OHOS {
namespace IntellVoiceUtils {
AutoTimer::AutoTimer()
{
    Reset();
}

AutoTimer::AutoTimer(const std::string &logInfo) : logInfo_(logInfo)
{
    Reset();
}

AutoTimer::~AutoTimer()
{
    if (isReset_) {
        PrintTimeElapse();
    }
}

void AutoTimer::PrintTimeElapse()
{
    PrintTimeElapse(logInfo_);
}

void AutoTimer::PrintTimeElapse(const std::string &logInfo)
{
    std::string log;

    try {
        if (!logInfo.empty()) {
            log += logInfo + " ";
        } else {
            log += logInfo_ + " ";
        }

        std::ostringstream ss;
        ss << TimeElapseMs();
        log += "time elapse: " + ss.str() + "ms";
    } catch (const std::length_error& err) {
        INTELL_VOICE_LOG_ERROR("length error");
        return;
    }

    INTELL_VOICE_LOG_DEBUG("%{public}s", log.c_str());

    isReset_ = false;
}

void AutoTimer::Reset()
{
    TimeUtil::GetTime(timeStart_);
    isReset_ = true;
}

long AutoTimer::TimeElapseUs()
{
    isReset_ = false;

    timespec timeEnd;
    TimeUtil::GetTime(timeEnd);

    return TimeUtil::TimeElapseUs(timeStart_, timeEnd);
}

uint32_t AutoTimer::TimeElapseMs()
{
    return TimeElapseUs() / MS_PER_US;
}

uint32_t AutoTimer::TimeElapseS()
{
    return TimeElapseUs() / (S_PER_MS * MS_PER_US);
}

string TimeUtil::GetCurrTime(TimeFormat format)
{
    time_t rawTime;
    struct tm *timeInfo = nullptr;
    char buffer[100] = { 0 };

    time(&rawTime);
    timeInfo = localtime(&rawTime);
    if (timeInfo != nullptr) {
        if (format == TIME_FORMAT_DEFAULT) {
            strftime(buffer, sizeof(buffer), "%Y_%m_%d_%H_%M_%S_", timeInfo);
        } else if (format == TIME_FORMAT_CONTINOUS) {
            strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", timeInfo);
        } else if (format == TIME_FORMAT_STANDARD) {
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
        } else {
            INTELL_VOICE_LOG_WARN("invalid format:%{public}d", format);
        }
    }
    std::string str(buffer);

    return str;
}

string TimeUtil::GetCurrTimeUs()
{
    struct timeval tv;
    char buffer[100] = {0};
    if (gettimeofday(&tv, nullptr) == -1) {
        INTELL_VOICE_LOG_ERROR("get time of day error");
        return "";
    }
    struct tm *timeInfo = localtime(&tv.tv_sec);
    if (timeInfo != nullptr) {
        strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", timeInfo);
    }
    string str(buffer);
    stringstream ss;
    ss << tv.tv_usec;
    str += ss.str();
    return str;
}

time_t TimeUtil::GetFormatTimeToSec(const string &formatTime)
{
    INTELL_VOICE_LOG_DEBUG("GetFormatTimeToSec, formatTime is %{public}s", formatTime.c_str());
    struct tm s_tm;

    if (!strptime(formatTime.c_str(), "%Y%m%d%H%M%S", &s_tm)) {
        INTELL_VOICE_LOG_ERROR("get file create time error");
        return INVALID_TIME_T;
    }

    return mktime(&s_tm);
}

bool TimeUtil::IsFormatTimeExpired(const string &formatTime, int maxKeepTime)
{
    time_t currentTime;
    time_t originalTime;

    currentTime = time(nullptr);
    originalTime = GetFormatTimeToSec(formatTime);
    if ((originalTime == INVALID_TIME_T) || (currentTime == INVALID_TIME_T)) {
        INTELL_VOICE_LOG_ERROR("get sys time error");
        return false;
    }

    return (currentTime >= originalTime) && (currentTime - originalTime >= maxKeepTime);
}

uint64_t TimeUtil::GetCurrentTimeMs()
{
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}
}
}
