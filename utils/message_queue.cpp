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
Message::Message(uint32_t what)
{
    mWhat = what;
    arg1 = 0;
    arg2 = 0;
}

Message::Message(uint32_t what, int32_t arg1)
{
    mWhat = what;
    this->arg1 = arg1;
}

Message::Message(uint32_t what, int32_t arg1, int32_t arg2, float arg3)
{
    mWhat = what;
    this->arg1 = arg1;
    this->arg2 = arg2;
    this->arg3 = arg3;
}

Message::Message(uint32_t what, int32_t arg1, int32_t arg2, const std::string &obj)
{
    mWhat = what;
    this->arg1 = arg1;
    this->arg2 = arg2;
    this->obj = obj;
}

Message::Message(uint32_t what, int32_t arg1, int32_t arg2, float arg3, const std::string &obj)
{
    mWhat = what;
    this->arg1 = arg1;
    this->arg2 = arg2;
    this->arg3 = arg3;
    this->obj = obj;
}

Message::Message(uint32_t what, std::shared_ptr<void> obj2)
{
    mWhat = what;
    this->obj2 = obj2;
}

Message::Message(uint32_t what, std::shared_ptr<void> obj2, std::shared_ptr<void> obj3)
{
    mWhat = what;
    this->obj2 = obj2;
    this->obj3 = obj3;
}

Message::Message(uint32_t what, void* voidPtr)
{
    mWhat = what;
    this->voidPtr = voidPtr;
}

Message::Message(uint32_t what, void* voidPtr, int32_t arg1)
{
    mWhat = what;
    this->voidPtr = voidPtr;
    this->arg1 = arg1;
}

Message::~Message()
{
    voidPtr = nullptr;
}

MessageQueue::MessageQueue(uint32_t size) : mSize(size), mLock(PTHREAD_MUTEX_INITIALIZER)
{
    pthread_condattr_t attr;
    int result = pthread_condattr_init(&attr);
    result += pthread_cond_init(&mCond, &attr);
    result += pthread_condattr_destroy(&attr);
    if (result != 0) {
        INTELL_VOICE_LOG_ERROR("message queue construct init cond failed");
    }
}

MessageQueue::~MessageQueue()
{
    int result = pthread_mutex_destroy(&mLock);
    result += pthread_cond_destroy(&mCond);
    if (result != 0) {
        INTELL_VOICE_LOG_ERROR("message queue deconstruct destroy cond failed");
    }
}

shared_ptr<Message> MessageQueue::ReceiveMsg()
{
    shared_ptr<Message> msg = nullptr;
    pthread_mutex_lock(&mLock);
    while (mQueue.empty()) {
        pthread_cond_wait(&mCond, &mLock);
    }

    msg = mQueue.front();
    mQueue.pop();
    pthread_mutex_unlock(&mLock);

    return msg;
}

bool MessageQueue::SendMsg(shared_ptr<Message> msg)
{
    pthread_mutex_lock(&mLock);
    if (static_cast<uint32_t>(mQueue.size()) >= mSize || msg == nullptr) {
        INTELL_VOICE_LOG_WARN("send message failed, msg queue full(qsize %{public}d)", static_cast<int>(mQueue.size()));
        pthread_mutex_unlock(&mLock);
        return false;
    }

    try {
        mQueue.push(msg);
    } catch (const std::length_error& err) {
        INTELL_VOICE_LOG_ERROR("messagequeue push, length error");
        pthread_mutex_unlock(&mLock);
        return false;
    }

    pthread_cond_signal(&mCond);
    pthread_mutex_unlock(&mLock);

    return true;
}

void MessageQueue::Clear()
{
    pthread_mutex_lock(&mLock);
    while (!mQueue.empty()) {
        mQueue.pop();
    }
    pthread_mutex_unlock(&mLock);
}
}
}
