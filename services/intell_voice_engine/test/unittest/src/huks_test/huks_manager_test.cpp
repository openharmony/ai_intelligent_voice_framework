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

#include "huks_manager_test.h"

#include "intell_voice_log.h"
#include "huks_aes_adapter.h"

#define LOG_TAG "HuksManagerTest"

using namespace std;
using namespace OHOS::IntellVoiceUtils;

namespace OHOS {
namespace IntellVoiceEngine {

HuksManagerTest::HuksManagerTest() {}

HuksManagerTest::~HuksManagerTest() {}

bool HuksManagerTest::Encrypt(std::unique_ptr<Uint8ArrayBuffer> &inData, std::unique_ptr<Uint8ArrayBuffer> &outData)
{
    if (inData == nullptr) {
        INTELL_VOICE_LOG_ERROR("in data failed");
        return false;
    }
    HuksAesAdapter::Encrypt(inData, outData);
    return true;
}

bool HuksManagerTest::Decrypt(std::unique_ptr<Uint8ArrayBuffer> &inData, std::unique_ptr<Uint8ArrayBuffer> &outData)
{
    if (inData == nullptr) {
        INTELL_VOICE_LOG_ERROR("in data failed");
        return false;
    }
    HuksAesAdapter::Decrypt(inData, outData);
    return true;
}
}  // namespace IntellVoiceEngine
}  // namespace OHOS
