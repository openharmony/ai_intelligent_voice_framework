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

#include "task_manager_test.h"

#include "intell_voice_log.h"

#define LOG_TAG "TaskManagerTest"

using namespace std;
using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {
static constexpr uint32_t MAX_TASK_NUM = 5;
static const std::string TASK_THREAD_NAME = "TaskThreadTest";

TaskManagerTest::TaskManagerTest() : TaskExecutor(TASK_THREAD_NAME, MAX_TASK_NUM)
{
    TaskExecutor::StartThread();
}

TaskManagerTest::~TaskManagerTest()
{
    TaskExecutor::StopThread();
}

void TaskManagerTest::AddAsyncTask(int32_t value)
{
    TaskExecutor::AddAsyncTask(
        [this, value]() {
            sleep(1);
            threadId_ = value;
            INTELL_VOICE_LOG_INFO("AddAsyncTask threadId_: %{public}d", threadId_);
        },
        std::to_string(value), false);
}

void TaskManagerTest::AddSyncTask(int32_t value)
{
    TaskExecutor::AddSyncTask([this, value]() {
        sleep(1);
        threadId_ = value;
        INTELL_VOICE_LOG_INFO("AddSyncTask threadId_: %{public}d", threadId_);
    });
}
}  // namespace IntellVoiceEngine
}  // namespace OHOS
