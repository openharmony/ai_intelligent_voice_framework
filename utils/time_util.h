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
#ifndef TIME_UTIL_H
#define TIME_UTIL_H

#include <string>
#include <iostream>
#include "base_constants.h"

namespace OHOS {
namespace IntellVoiceUtils {
enum TimeFormat {
    TIME_FORMAT_DEFAULT = 0,
    TIME_FORMAT_CONTINOUS,
    TIME_FORMAT_STANDARD,
    TIME_FORMAT_NUM
};

class AutoTimer {
public:
    AutoTimer();
    explicit AutoTimer(const std::string &logInfo);
    virtual ~AutoTimer();

    void PrintTimeElapse();
    void PrintTimeElapse(const std::string &logInfo);
    void Reset();
    long TimeElapseUs();
    uint32_t TimeElapseMs();
    uint32_t TimeElapseS();

private:
    std::string logInfo_;
    timespec timeStart_ { 0, 0 };
    bool isReset_ { true };
};

class TimeUtil {
public:
    TimeUtil() {}
    ~TimeUtil() {}
    static std::string GetCurrTime(TimeFormat format = TIME_FORMAT_DEFAULT);
    static std::string GetCurrTimeUs();
    static time_t GetFormatTimeToSec(const std::string &formatTime);
    static bool IsFormatTimeExpired(const std::string &formatTime, int maxKeepTime);
    static void GetTime(timespec &start);
    static uint32_t TimeElapse(const timespec &start);
    static void TimeElapse(const timespec &start, const timespec &end);
    static long TimeElapseUs(const timespec &start, const timespec &end);
    static uint64_t GetCurrentTimeMs();
};

inline void TimeUtil::GetTime(timespec &start)
{
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        return;
    }
}

inline uint32_t TimeUtil::TimeElapse(const timespec &start)
{
    timespec current;
    if (clock_gettime(CLOCK_REALTIME, &current) == -1) {
        return 0;
    }

    return (current.tv_sec > start.tv_sec) ? (current.tv_sec - start.tv_sec) : 0;
}

inline void TimeUtil::TimeElapse(const timespec &start, const timespec &end)
{
    long secs = end.tv_sec - start.tv_sec;
    long hour = secs / (H_PER_MIN * MIN_PER_S);
    long min = (secs % (H_PER_MIN * MIN_PER_S)) / MIN_PER_S;
    long sec = secs % MIN_PER_S;

    std::cout << hour << ":" << min << ":" << sec << std::endl;
    return;
}

inline long TimeUtil::TimeElapseUs(const timespec &start, const timespec &end)
{
    long usecs = MS_PER_US * S_PER_MS * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / US_PER_NS;
    return usecs;
}
}
}

#endif
