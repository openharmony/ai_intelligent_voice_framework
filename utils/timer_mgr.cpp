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

const int32_t US_PER_MS = 1000;

void LogTime(const string& prefix)
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
    strftime(tmbuf, sizeof(tmbuf), "%Y-%m-%d %H:%M:%S", nowtm);
    INTELL_VOICE_LOG_INFO("%s %s.%lld", prefix.c_str(), tmbuf, static_cast<long long>(now.tv_usec));
}

int64_t NowTimeUs()
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) < 0) {
        INTELL_VOICE_LOG_ERROR("clock gettime failed");
        return 0;
    }
    return t.tv_sec * 1000000LL + t.tv_nsec / 1000LL;
}

TimerItem::TimerItem(int id, int type, int cookie, int64_t delayUs, ITimerObserver* observer)
    : timerId(id), type(type), cookie(cookie), observer(observer)
{
    tgtUs = NowTimeUs() + delayUs;
}

TimerMgr::TimerMgr(int maxTimerNum) : IdAllocator(maxTimerNum), status(TIMER_STATUS_INIT), timerObserver(nullptr)
{
}

TimerMgr::~TimerMgr()
{
    Stop();
}

void TimerMgr::Start(ITimerObserver* observer)
{
    unique_lock<mutex> lock(mTimeMutex);

    if (status == TIMER_STATUS_STARTED) {
        return;
    }

    if (status != TIMER_STATUS_INIT) {
        INTELL_VOICE_LOG_ERROR("status is not init");
        return;
    }

    mWorkThread = thread(&TimerMgr::WorkThread, this);

    timerObserver = observer;
    status = TIMER_STATUS_STARTED;
    LogTime("start timermgr");
}

void TimerMgr::Stop()
{
    {
        lock_guard<mutex> lock(mTimeMutex);
        if (status != TIMER_STATUS_STARTED) {
            INTELL_VOICE_LOG_ERROR("status is not started");
            return;
        }
        status = TIMER_STATUS_TO_QUIT;
        LogTime("stop timermgr");
        mCV.notify_all();
    }

    if (mWorkThread.joinable()) {
        mWorkThread.join();
    }

    Clear();
}

int TimerMgr::SetTimer(int type, int64_t delayUs, int cookie, ITimerObserver* currObserver)
{
    unique_lock<mutex> lock(mTimeMutex);

    if (status != TIMER_STATUS_STARTED) {
        INTELL_VOICE_LOG_ERROR("timer mgr not started");
        return INVALID_ID;
    }

    ITimerObserver* observer = currObserver == nullptr ? timerObserver : currObserver;
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

    auto it = itemQueue.begin();
    while (it != itemQueue.end() && (*it)->tgtUs < addItem->tgtUs) {
        ++it;
    }

    bool needNotify = (it == itemQueue.begin());
    itemQueue.insert(it, addItem);

    if (needNotify) {
        mCV.notify_one();
    }

    ostringstream oss;
    oss << "set timer id " << id << " type " << type << " delay " << delayUs << " cookie " << cookie << " time ";
    LogTime(oss.str());

    return id;
}

int TimerMgr::ResetTimer(int timerId, int type, int64_t delayUs, int cookie, ITimerObserver* currObserver)
{
    {
        unique_lock<mutex> lock(mTimeMutex);
        if (itemQueue.size() == 1) {
            auto it = itemQueue.begin();
            if ((*it)->timerId != timerId) {
                INTELL_VOICE_LOG_ERROR("id %d can not correspond with timerId %d", (*it)->timerId, timerId);
                return INVALID_ID;
            }
            (*it)->tgtUs = NowTimeUs() + delayUs;
            mCV.notify_one();
            return timerId;
        }
    }
    KillTimer(timerId);
    return SetTimer(type, delayUs, cookie, currObserver);
}

void TimerMgr::KillTimer(int& timerId)
{
    unique_lock<mutex> lock(mTimeMutex);
    INTELL_VOICE_LOG_INFO("kill timer %d", timerId);
    for (auto it = itemQueue.begin(); it != itemQueue.end(); it++) {
        shared_ptr<TimerItem> curItem = *it;
        if (curItem->timerId == timerId) {
            INTELL_VOICE_LOG_INFO("kill timer id %d type %d path %d", timerId, curItem->type, curItem->cookie);

            ReleaseId(curItem->timerId);
            itemQueue.erase(it);
            timerId = INVALID_ID;
            break;
        }
    }
}

void TimerMgr::Clear()
{
    lock_guard<mutex> lock(mTimeMutex);

    itemQueue.clear();
    IdAllocator::ClearId();

    status = TIMER_STATUS_INIT;
    timerObserver = nullptr;
}

void TimerMgr::WorkThread()
{
    while (true) {
        TimerItem item;
        {
            unique_lock<mutex> lock(mTimeMutex);

            if (status != TIMER_STATUS_STARTED) {
                break;
            }

            if (itemQueue.empty()) {
                mCV.wait(lock);
                continue;
            }

            item = *itemQueue.front();
            int64_t now = NowTimeUs();
            if (now < item.tgtUs) {
                mCV.wait_for(lock, chrono::microseconds(item.tgtUs - now));
                continue;
            }
            ReleaseId(item.timerId);
            itemQueue.pop_front();
        }

        if (item.observer != nullptr) {
            TimerEvent info(item.type, item.timerId, item.cookie);
            item.observer->OnTimerEvent(info);
        }
    };

    INTELL_VOICE_LOG_INFO("timer thread exit");
}

