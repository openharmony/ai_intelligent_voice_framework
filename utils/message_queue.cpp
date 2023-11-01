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
#include "message_queue.h"
#include "intell_voice_log.h"

#define LOG_TAG "MesaageQueue"

using namespace std;

namespace OHOS {
namespace IntellVoiceUtils {
Message::Message(uint32_t what) : what_(what), arg1_(0), arg2_(0)
{
}

Message::Message(uint32_t what, int32_t arg1) : what_(what), arg1_(arg1)
{
}

Message::Message(uint32_t what, int32_t arg1, int32_t arg2, float arg3)
    : what_(what), arg1_(arg1), arg2_(arg2), arg3_(arg3)
{
}

Message::Message(uint32_t what, int32_t arg1, int32_t arg2, const std::string &obj)
    : what_(what), arg1_(arg1), arg2_(arg2), obj_(obj)
{
}

Message::Message(uint32_t what, int32_t arg1, int32_t arg2, float arg3, const std::string &obj)
    : what_(what), arg1_(arg1), arg2_(arg2), arg3_(arg3), obj_(obj)
{
}

Message::Message(uint32_t what, std::shared_ptr<void> obj2) : what_(what), obj2_(obj2)
{
}

Message::Message(uint32_t what, std::shared_ptr<void> obj2, std::shared_ptr<void> obj3)
    : what_(what), obj2_(obj2), obj3_(obj3)
{
}

Message::Message(uint32_t what, void* voidPtr) : what_(what), voidPtr_(voidPtr)
{
}

Message::Message(uint32_t what, int32_t arg1, void* voidPtr) : what_(what), arg1_(arg1), voidPtr_(voidPtr)
{
}

Message::~Message()
{
    voidPtr_ = nullptr;
}

MessageQueue::MessageQueue(uint32_t size) : size_(size), lock_(PTHREAD_MUTEX_INITIALIZER)
{
    pthread_condattr_t attr;
    int result = pthread_condattr_init(&attr);
    result += pthread_cond_init(&cond_, &attr);
    result += pthread_condattr_destroy(&attr);
    if (result != 0) {
        INTELL_VOICE_LOG_ERROR("message queue construct init cond failed");
    }
}

MessageQueue::~MessageQueue()
{
    int result = pthread_mutex_destroy(&lock_);
    result += pthread_cond_destroy(&cond_);
    if (result != 0) {
        INTELL_VOICE_LOG_ERROR("message queue deconstruct destroy cond failed");
    }
}

shared_ptr<Message> MessageQueue::ReceiveMsg()
{
    shared_ptr<Message> msg = nullptr;
    pthread_mutex_lock(&lock_);
    while (queue_.empty()) {
        pthread_cond_wait(&cond_, &lock_);
    }

    msg = queue_.front();
    queue_.pop();
    pthread_mutex_unlock(&lock_);

    return msg;
}

bool MessageQueue::SendMsg(shared_ptr<Message> msg)
{
    pthread_mutex_lock(&lock_);
    if (static_cast<uint32_t>(queue_.size()) >= size_ || msg == nullptr) {
        INTELL_VOICE_LOG_WARN("send message failed, msg queue full(%{public}d)", static_cast<int>(queue_.size()));
        pthread_mutex_unlock(&lock_);
        return false;
    }

    try {
        queue_.push(msg);
    } catch (const std::length_error& err) {
        INTELL_VOICE_LOG_ERROR("messagequeue push, length error");
        pthread_mutex_unlock(&lock_);
        return false;
    }

    pthread_cond_signal(&cond_);
    pthread_mutex_unlock(&lock_);

    return true;
}

void MessageQueue::Clear()
{
    pthread_mutex_lock(&lock_);
    while (!queue_.empty()) {
        queue_.pop();
    }
    pthread_mutex_unlock(&lock_);
}
}
}
