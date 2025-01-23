/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "swing_service_wrapper.h"

#include <dlfcn.h>
#include "intell_voice_log.h"

#define LOG_TAG "SwingServiceWrapper"

namespace OHOS {
namespace IntellVoiceEngine {
static const std::string SWINGSERVICE_SO_PATH = "libswing_client.z.so";

int32_t SwingServiceWrapper::LoadSwingServiceLib()
{
    const char *strError = nullptr;
    swingServicePriv_.handle = dlopen(SWINGSERVICE_SO_PATH.c_str(), RTLD_LAZY);
    if (swingServicePriv_.handle == nullptr) {
        strError = dlerror();
        INTELL_VOICE_LOG_ERROR("dlopen err:%{public}s", strError);
        return -1;
    }

    (void)dlerror(); // clear existing error
    swingServicePriv_.getSwingServiceInst = reinterpret_cast<GetSwingServiceInstFunc>(dlsym(
        swingServicePriv_.handle, "GetSwingServiceInst"));
    if (swingServicePriv_.getSwingServiceInst == nullptr) {
        strError = dlerror();
        INTELL_VOICE_LOG_ERROR("dlsym getSwingServiceInst err:%{public}s", strError);
        dlclose(swingServicePriv_.handle);
        swingServicePriv_.handle = nullptr;
        return -1;
    }

    INTELL_VOICE_LOG_INFO("load swingService lib success");
    return 0;
}

void SwingServiceWrapper::UnloadSwingServiceLib()
{
    if (swingServicePriv_.handle != nullptr) {
        dlclose(swingServicePriv_.handle);
        swingServicePriv_.handle = nullptr;
    }
}

SwingServiceWrapper::SwingServiceWrapper()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (LoadSwingServiceLib() == 0) {
        inst_ = swingServicePriv_.getSwingServiceInst();
        if (inst_ == nullptr) {
            INTELL_VOICE_LOG_ERROR("failed to get swingService inst");
        }
    }
}

SwingServiceWrapper::~SwingServiceWrapper()
{
    INTELL_VOICE_LOG_INFO("enter");
    UnloadSwingServiceLib();
    inst_ = nullptr;
}

int32_t SwingServiceWrapper::SubscribeSwingEvent(std::string swingEventType,
    std::map<std::string, std::string> eventParams)
{
    INTELL_VOICE_LOG_INFO("enter");
    std::lock_guard<std::mutex> lock(mutex_);
    if (inst_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("inst is nullptr");
        return -1;
    }

    return inst_->SubscribeSwingEvent(swingEventType, eventParams);
}

int32_t SwingServiceWrapper::UnSubscribeSwingEvent(std::string swingEventType,
    std::map<std::string, std::string> eventParams)
{
    INTELL_VOICE_LOG_INFO("enter");
    std::lock_guard<std::mutex> lock(mutex_);
    if (inst_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("inst is nullptr");
        return -1;
    }

    return inst_->UnSubscribeSwingEvent(swingEventType, eventParams);
}
}  // namespace IntellVoice
}  // namespace OHOS
