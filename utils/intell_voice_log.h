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
#ifndef INTELL_VOICE_LOG_H
#define INTELL_VOICE_LOG_H

#include <cstdio>
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002105
#undef LOG_TAG

#define INTELL_VOICE_LOG_DEBUG(fmt, ...) \
    HILOG_DEBUG(LOG_CORE, "[%{public}s:%{public}d]: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define INTELL_VOICE_LOG_ERROR(fmt, ...) \
    HILOG_ERROR(LOG_CORE, "[%{public}s:%{public}d]: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define INTELL_VOICE_LOG_WARN(fmt, ...) \
    HILOG_WARN(LOG_CORE, "[%{public}s:%{public}d]: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define INTELL_VOICE_LOG_INFO(fmt, ...) \
    HILOG_INFO(LOG_CORE, "[%{public}s:%{public}d]: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define INTELL_VOICE_LOG_FATAL(fmt, ...) \
    HILOG_FATAL(LOG_CORE, "[%{public}s:%{public}d]: " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define CHECK_CONDITION_RETURN_VOID(condition, message) \
    do {                                           \
        if (condition) {       \
            INTELL_VOICE_LOG_ERROR(message);       \
            return;                                \
        }                                          \
    } while (0)

#define CHECK_CONDITION_RETURN_FALSE(condition, message) \
    do {                                           \
        if (condition) {       \
            INTELL_VOICE_LOG_ERROR(message);       \
            return false;                                \
        }                                          \
    } while (0)

#define CHECK_CONDITION_RETURN_RET(condition, result, message) \
    do {                                           \
        if (condition) {       \
            INTELL_VOICE_LOG_ERROR(message);       \
            return (result);                                \
        }                                          \
    } while (0)

#endif
