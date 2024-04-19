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
#include "timer_mgr.h"

#include <unistd.h>
#include <sys/time.h>
#include <sstream>
#include "intell_voice_log.h"

#define LOG_TAG "TimerMgr"

using namespace std;

namespace OHOS {
namespace IntellVoiceUtils {
static void LogTime(const string &prefix)
{
    struct timeval now;
    if (gettimeofday(&now, nullptr) < 0) {
        INTELL_VOICE_LOG_ERROR("gettimeoftoday time info error");
        return;
    }

    struct tm *nowtm = localtime(&now.tv_sec);
    if (nowtm == nullptr) {
        INTELL_VOICE_LOG_ERROR("nowtm is nullptr");
        return;
    }

    char tmbuf[64] = {0};
    if (strftime(tmbuf, sizeof(tmbuf), "%Y-%m-%d %H:%M:%S", nowtm)) {
        INTELL_VOICE_LOG_INFO("%{public}s %{public}s.%{public}lld",
            prefix.c_str(), tmbuf, static_cast<long long>(now.tv_usec));
    }
}

static int64_t NowTimeUs()
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) < 0) {
        INTELL_VOICE_LOG_ERROR("clock gettime failed");
        return 0;
    }
    return t.tv_sec * 1000000LL + t.tv_nsec / 1000LL;
}

TimerItem::TimerItem(int id, int type, int cookie, int64_t delayUs, ITimerObserver *observer)
    : timerId(id), type(type), cookie(cookie), observer(observer)
{
    tgtUs = NowTimeUs() + delayUs;
}

TimerMgr::TimerMgr(int maxTimerNum) : IdAllocator(maxTimerNum), status_(TimerStatus::TIMER_STATUS_INIT),
    timerObserver_(nullptr)
{
}

TimerMgr::~TimerMgr()
{
    Stop();
}

void TimerMgr::Start(const std::string &threadName, ITimerObserver *observer)
{
    unique_lock<mutex> lock(timeMutex_);

    if (status_ == TimerStatus::TIMER_STATUS_STARTED) {
        return;
    }

    if (status_ != TimerStatus::TIMER_STATUS_INIT) {
        INTELL_VOICE_LOG_ERROR("status is not init");
        return;
    }

    workThread_ = thread(&TimerMgr::WorkThread, this);
    pthread_setname_np(workThread_.native_handle(), threadName.c_str());

    timerObserver_ = observer;
    status_ = TimerStatus::TIMER_STATUS_STARTED;
    LogTime("start timermgr");
}

void TimerMgr::Stop()
{
    {
        lock_guard<mutex> lock(timeMutex_);
        if (status_ != TimerStatus::TIMER_STATUS_STARTED) {
            INTELL_VOICE_LOG_WARN("status is not started");
            return;
        }
        status_ = TimerStatus::TIMER_STATUS_TO_QUIT;
        LogTime("stop timermgr");
        cv_.notify_all();
    }

    if (workThread_.joinable()) {
        workThread_.join();
    }

    Clear();
}

int TimerMgr::SetTimer(int type, int64_t delayUs, int cookie, ITimerObserver *currObserver)
{
    unique_lock<mutex> lock(timeMutex_);

    if (status_ != TimerStatus::TIMER_STATUS_STARTED) {
        INTELL_VOICE_LOG_ERROR("timer mgr not started");
        return INVALID_ID;
    }

    ITimerObserver* observer = currObserver == nullptr ? timerObserver_ : currObserver;
    if (observer == nullptr) {
        INTELL_VOICE_LOG_ERROR("observer is null");
        return INVALID_ID;
    }

    int id = AllocId();
    if (id == INVALID_ID) {
        INTELL_VOICE_LOG_ERROR("no available id");
        return INVALID_ID;
    }

    shared_ptr<TimerItem> addItem = make_shared<TimerItem>(id, type, cookie, delayUs, observer);
    if (addItem == nullptr) {
        INTELL_VOICE_LOG_ERROR("no avaiable memory");
        ReleaseId(id);
        return INVALID_ID;
    }

    auto it = itemQueue_.begin();
    while (it != itemQueue_.end() && (*it)->tgtUs < addItem->tgtUs) {
        ++it;
    }

    bool needNotify = (it == itemQueue_.begin());
    itemQueue_.insert(it, addItem);

    if (needNotify) {
        cv_.notify_one();
    }

    ostringstream oss;
    oss << "set timer id " << id << " type " << type << " delay " << delayUs << " cookie " << cookie << " time ";
    LogTime(oss.str());

    return id;
}

int TimerMgr::ResetTimer(int timerId, int type, int64_t delayUs, int cookie, ITimerObserver *currObserver)
{
    {
        unique_lock<mutex> lock(timeMutex_);
        if (itemQueue_.size() == 1) {
            auto it = itemQueue_.begin();
            if ((*it)->timerId != timerId) {
                INTELL_VOICE_LOG_ERROR("id %{public}d can not correspond with timerId %{public}d",
                    (*it)->timerId, timerId);
                return INVALID_ID;
            }
            (*it)->tgtUs = NowTimeUs() + delayUs;
            cv_.notify_one();
            return timerId;
        }
    }
    KillTimer(timerId);
    return SetTimer(type, delayUs, cookie, currObserver);
}

void TimerMgr::KillTimer(int &timerId)
{
    unique_lock<mutex> lock(timeMutex_);
    INTELL_VOICE_LOG_INFO("kill timer %d", timerId);
    for (auto it = itemQueue_.begin(); it != itemQueue_.end(); it++) {
        shared_ptr<TimerItem> curItem = *it;
        if (curItem->timerId == timerId) {
            INTELL_VOICE_LOG_INFO("kill timer id:%{public}d, type: %{public}d, cookie:%{public}d",
                timerId, curItem->type, curItem->cookie);
            ReleaseId(curItem->timerId);
            itemQueue_.erase(it);
            timerId = INVALID_ID;
            break;
        }
    }

    if (timerId != INVALID_ID) {
        INTELL_VOICE_LOG_WARN("can not find timer id:%{public}d", timerId);
        timerId = INVALID_ID;
    }
}

void TimerMgr::Clear()
{
    lock_guard<mutex> lock(timeMutex_);

    itemQueue_.clear();
    IdAllocator::ClearId();

    status_ = TimerStatus::TIMER_STATUS_INIT;
    timerObserver_ = nullptr;
}

void TimerMgr::WorkThread()
{
    while (true) {
        TimerItem item;
        {
            unique_lock<mutex> lock(timeMutex_);

            if (status_ != TimerStatus::TIMER_STATUS_STARTED) {
                break;
            }

            if (itemQueue_.empty()) {
                cv_.wait(lock);
                continue;
            }

            item = *itemQueue_.front();
            int64_t now = NowTimeUs();
            if (now < item.tgtUs) {
                cv_.wait_for(lock, chrono::microseconds(item.tgtUs - now));
                continue;
            }
            ReleaseId(item.timerId);
            itemQueue_.pop_front();
        }

        if (item.observer != nullptr) {
            TimerEvent info(item.type, item.timerId, item.cookie);
            item.observer->OnTimerEvent(info);
        }
    };

    INTELL_VOICE_LOG_INFO("timer thread exit");
}
}
}
