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
#ifndef TIMER_MGR_H
#define TIMER_MGR_H

#include <memory>
#include <list>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "id_allocator.h"

namespace OHOS {
namespace IntellVoiceUtils {
constexpr int32_t US_PER_MS = 1000;
// struct for set timer
struct TimerCfg {
    explicit TimerCfg(int type = 0, int64_t delayUs = 0, int cookie = 0)
        : type(type), delayUs(delayUs), cookie(cookie)
    {
    };

    int type;
    int64_t delayUs;
    int cookie;
};

// struct for callback
struct TimerEvent {
    explicit TimerEvent(int type = 0, int timeId = 0, int cookie = 0) : type(type), timeId(timeId), cookie(cookie) {};

    int type;
    int timeId;
    int cookie;
};


struct ITimerObserver {
    virtual ~ITimerObserver() {};
    virtual void OnTimerEvent(TimerEvent &info) = 0;
};

enum class TimerStatus {
    TIMER_STATUS_INIT,
    TIMER_STATUS_STARTED,
    TIMER_STATUS_TO_QUIT
};

struct TimerItem {
    explicit TimerItem(int id = 0, int type = 0, int cookie = 0,
        int64_t delayUs = static_cast<long long>(0), ITimerObserver *observer = nullptr);
    bool operator==(const TimerItem& right) const
    {
        return tgtUs == right.tgtUs;
    };
    bool operator<(const TimerItem& right) const
    {
        return tgtUs < right.tgtUs;
    };

    int timerId;
    int type;
    int cookie;
    int64_t tgtUs;
    ITimerObserver *observer;
};

class TimerMgr : private IdAllocator {
public:
    explicit TimerMgr(int maxTimerNum = 10);
    ~TimerMgr() override;

    void Start(const std::string &threadName, ITimerObserver *observer = nullptr);
    void Stop();
    int SetTimer(int type, int64_t delayUs, int cookie = 0, ITimerObserver *currObserver = nullptr);
    int ResetTimer(int timerId, int type, int64_t delayUs, int cookie, ITimerObserver *currObserver);
    void KillTimer(int &timerId);

private:
    void Clear();
    void WorkThread();

private:
    TimerStatus status_;
    ITimerObserver *timerObserver_;
    std::list<std::shared_ptr<TimerItem>> itemQueue_;

    std::mutex timeMutex_;
    std::condition_variable cv_;
    std::thread workThread_;
};
}
}
#endif
