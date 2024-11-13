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
#ifndef WAKEUP_UNIT_TEST_H
#define WAKEUP_UNIT_TEST_H

#include <gtest/gtest.h>
#include <unistd.h>
#include <cstdio>
#include <refbase.h>
#include "intell_voice_update_callback.h"

namespace OHOS {
namespace IntellVoiceTests {
class UpdateEngineTest : public testing::Test, public RefBase {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    std::shared_ptr<IntellVoiceUpdateCallback> cb_ = nullptr;
};
}  // namespace IntellVoiceTests
}  // namespace OHOS
#endif