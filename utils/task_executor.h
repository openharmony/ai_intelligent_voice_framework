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
#ifndef INTELL_VOICE_TASK_EXECUTOR_H
#define INTELL_VOICE_TASK_EXECUTOR_H
#include <mutex>
#include <thread>
#include <memory>
#include <functional>
#include <future>
#include <pthread.h>
#include <sys/types.h>
#include "queue_util.h"

namespace OHOS {
namespace IntellVoiceUtils {
class TaskExecutor : private QueueUtil<std::function<void()>> {
public:
    TaskExecutor(const std::string &threadName, uint32_t capacity);
    ~TaskExecutor();
    void StartThread();
    void StopThread();
    template <typename F>
    void AddAsyncTask(F &&func)
    {
        auto task = std::make_shared<std::packaged_task<void()>>(std::forward<F>(func));
        Push([task]() { (*task)(); });
    }
    template <typename F, typename... Args>
    auto AddSyncTask(F &&func, Args &&...args) -> decltype(func(args...))
    {
        auto task = std::make_shared<std::packaged_task<decltype(func(args...))()>>(std::bind(
            std::forward<F>(func), std::forward<Args>(args)...));
        auto ret = task->get_future();
        Push([task]() { (*task)(); });

        ret.wait();
        return ret.get();
    }

private:
    static void *ExecuteInThread(void *arg);

private:
    std::mutex mutex_;
    std::string threadName_;
    pthread_t tid_ = 0;
    bool isRuning_ = false;
};
}
}
#endif