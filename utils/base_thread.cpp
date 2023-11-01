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
#include "base_thread.h"
#include <unistd.h>
#include "intell_voice_log.h"

#define LOG_TAG "BaseThread"

using namespace std;

namespace OHOS {
namespace IntellVoiceUtils {
BaseThread::BaseThread() : tid_(0), isRuning_(false)
{
}

BaseThread::~BaseThread()
{
    if (isRuning_) {
        pthread_detach(tid_);
    }
}

void *BaseThread::RunInThread(void *arg)
{
    BaseThread *pt = static_cast<BaseThread *>(arg);
    pt->Run();

    return nullptr;
}

void BaseThread::Start()
{
    std::lock_guard<std::mutex> lock(mutex_);

    int ret = pthread_create(&tid_, nullptr, BaseThread::RunInThread, this);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("create thread failed");
        return;
    }

    isRuning_ = true;
}

void BaseThread::Join()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!isRuning_) {
        return;
    }

    pthread_join(tid_, nullptr);
    isRuning_ = false;
}

bool BaseThread::IsRuning() const
{
    return isRuning_;
}

pid_t BaseThread::Gettid() const
{
    return tid_;
}
}
}
