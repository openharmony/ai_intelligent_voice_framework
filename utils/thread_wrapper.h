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
#ifndef THREAD_WRAPPER_H
#define THREAD_WRAPPER_H

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include "nocopyable.h"
#include "ffrt_api.h"

namespace OHOS {
namespace IntellVoiceUtils {
enum class TaskQoS {
    INHERENT = 0,
    BACKGROUND,
    UTILITY,
    DEFAULT,
    USER_INITIATED,
    DEADLINE_REQUEST,
    USER_INTERACTIVE
};

class ThreadWrapper {
public:
    ThreadWrapper() = default;
    virtual ~ThreadWrapper();
    bool Start(const std::string &name, TaskQoS qos = TaskQoS::DEFAULT);
    void Join();

protected:
    virtual void Run() = 0;

private:
    void RunInThread();
#ifdef USE_FFRT
    ffrt::qos Convert2FfrtQos(TaskQoS taskqos);
#endif

private:
    std::unique_ptr<ffrt::thread> thread_ = nullptr;
    ffrt::mutex mutex_;

    DISALLOW_COPY(ThreadWrapper);
    DISALLOW_MOVE(ThreadWrapper);
};
}
}
#endif