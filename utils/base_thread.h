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
#ifndef BASE_THREAD_H
#define BASE_THREAD_H

#include <pthread.h>
#include <iostream>
#include <sys/types.h>
#include <mutex>

namespace OHOS {
namespace IntellVoiceUtils {
class BaseThread {
public:
    BaseThread();
    virtual ~BaseThread();

    void Start();
    void Join();
    bool IsRuning() const;

    pid_t Gettid() const;

protected:
    virtual void Run() = 0;

private:
    static void *RunInThread(void *arg);
    pthread_t tid_;
    bool isRuning_;
    std::mutex mutex_ {};
};
}
}
#endif
