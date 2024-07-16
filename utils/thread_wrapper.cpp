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
#include "thread_wrapper.h"
#include "intell_voice_log.h"

#define LOG_TAG "ThreadWrapper"

namespace OHOS {
namespace IntellVoiceUtils {
ThreadWrapper::~ThreadWrapper()
{
    Join();
}

bool ThreadWrapper::Start(const std::string &name, TaskQoS qos)
{
    std::unique_lock uniqueLock(mutex_);
    if (thread_ != nullptr) {
        INTELL_VOICE_LOG_WARN("thread is already started, name:%{public}s", name.c_str());
        return true;
    }

#ifdef USE_FFRT
    INTELL_VOICE_LOG_INFO("use ffrt");
    thread_ = std::make_unique<ffrt::thread>(name.c_str(), Convert2FfrtQos(qos), &ThreadWrapper::RunInThread, this);
#else
    INTELL_VOICE_LOG_INFO("do not use ffrt");
    thread_ = std::make_unique<std::thread>(&ThreadWrapper::RunInThread, this);
#endif

    if (thread_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("failed to create thread");
        return false;
    }

#ifndef USE_FFRT
    pthread_setname_np(thread_->native_handle(), name.c_str());
#endif
    return true;
}

void ThreadWrapper::Join()
{
    std::unique_lock uniqueLock(mutex_);
    if (thread_ == nullptr) {
        return;
    }
    if (thread_->joinable()) {
        uniqueLock.unlock();
        thread_->join();
        uniqueLock.lock();
    }
    thread_.reset();
}

void ThreadWrapper::RunInThread()
{
    Run();
}

#ifdef USE_FFRT
ffrt::qos ThreadWrapper::Convert2FfrtQos(TaskQoS taskqos)
{
    switch (taskqos) {
        case TaskQoS::INHERENT:
            return ffrt::qos_inherit;
        case TaskQoS::BACKGROUND:
            return ffrt::qos_background;
        case TaskQoS::UTILITY:
            return ffrt::qos_utility;
        case TaskQoS::DEFAULT:
            return ffrt::qos_default;
        case TaskQoS::USER_INITIATED:
            return ffrt::qos_user_initiated;
        case TaskQoS::DEADLINE_REQUEST:
            return ffrt::qos_deadline_request;
        case TaskQoS::USER_INTERACTIVE:
            return ffrt::qos_user_interactive;
        default:
            break;
    }

    return ffrt::qos_inherit;
}
#endif
}
}