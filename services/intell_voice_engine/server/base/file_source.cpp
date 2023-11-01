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
#include "file_source.h"

#include "securec.h"
#include "intell_voice_log.h"
#include "memory_guard.h"

using namespace OHOS::IntellVoiceUtils;
#define LOG_TAG "FileSource"

namespace OHOS {
namespace IntellVoiceEngine {
FileSource::FileSource(uint32_t minBufferSize, uint32_t bufferCnt, const std::string &filePath,
    std::unique_ptr<FileSourceListener> listener)
    : minBufferSize_(minBufferSize), bufferCnt_(bufferCnt), filePath_(filePath), listener_(std::move(listener))
{
}

FileSource::~FileSource()
{
    Stop();
}

bool FileSource::Start()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (listener_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("listener_ is nullptr");
        return false;
    }

    if (minBufferSize_ == 0) {
        INTELL_VOICE_LOG_ERROR("minBufferSize_ is invalid");
        return false;
    }

    buffer_ = std::shared_ptr<uint8_t>(new uint8_t[minBufferSize_], [](uint8_t *p) { delete[] p; });
    if (buffer_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("malloc buffer failed");
        return false;
    }

    fileIn_ = std::make_unique<std::ifstream>(filePath_, std::ios::binary);
    if (fileIn_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("open input file failed");
        return false;
    }

    fileIn_->seekg(0, fileIn_->end);
    uint32_t size = static_cast<uint32_t>(fileIn_->tellg());
    if (size < minBufferSize_ * bufferCnt_) {
        INTELL_VOICE_LOG_ERROR("file size:%{public}u is smaller than required", size);
        fileIn_->close();
        fileIn_ = nullptr;
        return false;
    }

    isReading_.store(true);
    std::thread t1(std::bind(&FileSource::ReadThread, this));
    readThread_ = std::move(t1);
    return true;
}

void FileSource::ReadThread()
{
    uint32_t readCnt = 0;
    bool isError = true;
    if (fileIn_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("fileIn_ is nullptr");
        return;
    }

    fileIn_->seekg(0, fileIn_->beg);

    while (isReading_.load()) {
        if (readCnt >= bufferCnt_) {
            INTELL_VOICE_LOG_INFO("finish reading data");
            isError = false;
            break;
        }

        if (!(fileIn_->read(reinterpret_cast<char *>(buffer_.get()), minBufferSize_))) {
            INTELL_VOICE_LOG_ERROR("failed to read file");
            break;
        }

        if (listener_ != nullptr) {
            listener_->fileBufferCb_(buffer_.get(), minBufferSize_);
        }

        ++readCnt;
    }

    if (listener_ != nullptr) {
        listener_->fileEndCb_(isError);
    }
}

void FileSource::Stop()
{
    INTELL_VOICE_LOG_INFO("enter");
    MemoryGuard memoryGuard;
    if (isReading_.load()) {
        isReading_.store(false);
        readThread_.join();
    }

    if (fileIn_ != nullptr) {
        fileIn_->close();
        fileIn_ = nullptr;
    }

    buffer_ = nullptr;
    listener_ = nullptr;
}
}
}