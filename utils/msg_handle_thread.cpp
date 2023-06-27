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
#include "msg_handle_thread.h"
#include <sys/types.h>
#include <unistd.h>
#include "intell_voice_log.h"

#define LOG_TAG "MsgHandleThread"

using namespace std;

namespace OHOS {
namespace IntellVoiceUtils {
static const uint32_t MSQ_QUEUE_MAX_LEN = 100;
static const int32_t MSG_MAX_SYNC_TIMEOUT = 5;

MsgHandleThread::MsgHandleThread() : msgQue(MSQ_QUEUE_MAX_LEN), callbackThread(nullptr) {}

MsgHandleThread::MsgHandleThread(std::shared_ptr<MessageQueue> callbackMsgQue)
    : callbackMsgQue(callbackMsgQue), msgQue(MSQ_QUEUE_MAX_LEN), callbackThread(nullptr)
{}

MsgHandleThread::MsgHandleThread(MsgHandleThread *callbackThread)
    : msgQue(MSQ_QUEUE_MAX_LEN), callbackThread(callbackThread)
{}

MsgHandleThread::~MsgHandleThread() {}

void MsgHandleThread::SetCallbackThread(MsgHandleThread *tmpCallbackThread)
{
    callbackThread = tmpCallbackThread;
}

// the default realization is for debug, subclass should override this func
bool MsgHandleThread::HandleMsg(Message &msg)
{
    INTELL_VOICE_LOG_INFO("run thread %u process msg %u", Gettid(), msg.mWhat);

    SendbackMsg(msg);

    return true;
}

bool MsgHandleThread::SendMsg(Message msg)
{
    try {
        msgQue.SendMsg(std::make_shared<Message>(msg));
    } catch (const std::length_error& err) {
        INTELL_VOICE_LOG_ERROR("length error");
        return false;
    }

    return true;
}

bool MsgHandleThread::SendMsg(std::shared_ptr<Message> msg)
{
    if (msg == nullptr) {
        return false;
    }

    msgQue.SendMsg(msg);
    return true;
}

bool MsgHandleThread::SendSynMsg(shared_ptr<Message> msg)
{
    if (msg == nullptr) {
        return false;
    }

    msg->result = std::make_shared<SynInfo>();
    if (msg->result == nullptr) {
        INTELL_VOICE_LOG_ERROR("create sync info failed");
        return false;
    }

    unique_lock<mutex> lock(msg->result->mMutex);
    msgQue.SendMsg(msg);
    if (msg->result->mCV.wait_for(lock, chrono::seconds(MSG_MAX_SYNC_TIMEOUT)) == std::cv_status::no_timeout) {
        return true;
    } else {
        INTELL_VOICE_LOG_WARN("send syn msg timeout");
        return false;
    }
}

void MsgHandleThread::SendbackMsg(Message msg)
{
    if (callbackThread != nullptr) {
        callbackThread->SendMsg(msg);
    }

    if (callbackMsgQue != nullptr) {
        callbackMsgQue->SendMsg(make_shared<Message>(msg));
    }
}

void MsgHandleThread::Run()
{
    bool isQuit = false;

    while (!isQuit) {
        shared_ptr<Message> msg = msgQue.ReceiveMsg();

        isQuit = HandleMsg(*msg);

        if (msg->result != nullptr) {
            unique_lock<mutex> lock(msg->result->mMutex);
            msg->result->mCV.notify_all();
        }
    }
}

string MsgHandleThread::ToString() const
{
    return "";
}
}
}
