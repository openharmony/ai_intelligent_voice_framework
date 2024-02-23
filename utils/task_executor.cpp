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
#include "task_executor.h"
#include <sys/prctl.h>
#include "intell_voice_log.h"

#define LOG_TAG "TaskExecutor"

namespace OHOS {
namespace IntellVoiceUtils {
TaskExecutor::TaskExecutor(const std::string &threadName, uint32_t capacity) : threadName_(threadName)
{
    Init(capacity);
}

TaskExecutor::~TaskExecutor()
{
    StopThread();
}

void TaskExecutor::StartThread()
{
    std::lock_guard<std::mutex> lock(mutex_);
    int ret = pthread_create(&tid_, nullptr, TaskExecutor::ExecuteInThread, this);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("create thread failed");
        return;
    }

    isRuning_ = true;
}

void TaskExecutor::StopThread()
{
    Uninit();
    std::lock_guard<std::mutex> lock(mutex_);
    if (!isRuning_) {
        return;
    }

    pthread_join(tid_, nullptr);
    isRuning_ = false;
}

void *TaskExecutor::ExecuteInThread(void *arg)
{
    TaskExecutor *executor = static_cast<TaskExecutor *>(arg);
    if (executor == nullptr) {
        INTELL_VOICE_LOG_ERROR("executor is nullptr");
        return nullptr;
    }
    prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(executor->threadName_.c_str()), 0, 0, 0);
    do {
        std::function<void()> task;
        if (!executor->Pop(task)) {
            INTELL_VOICE_LOG_INFO("no task needed to execute");
            break;
        }
        task();
    } while (1);
    return nullptr;
}
}
}