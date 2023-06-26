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
#ifndef FILE_SOURCE_H
#define FILE_SOURCE_H

#include <memory>
#include <thread>
#include <functional>
#include <atomic>
#include <fstream>
#include <string>

namespace OHOS {
namespace IntellVoiceEngine {
using OnFileBufferCb = std::function<void(uint8_t *buffer, uint32_t size)>;
using OnFileEndCb = std::function<void(bool isError)>;

struct FileSourceListener {
    FileSourceListener(OnFileBufferCb fileBufferCb, OnFileEndCb fileEndCb)
        : fileBufferCb_(fileBufferCb), fileEndCb_(fileEndCb) {}
    OnFileBufferCb fileBufferCb_;
    OnFileEndCb fileEndCb_;
};

class FileSource {
public:
    FileSource(uint32_t minBufferSize, uint32_t bufferCnt, const std::string &filePath,
        std::unique_ptr<FileSourceListener> listener);
    ~FileSource();
    bool Start();
    void Stop();

private:
    void ReadThread();
    bool Read();

private:
    uint32_t minBufferSize_ = 0;
    uint32_t bufferCnt_ = 0;
    std::string filePath_;
    std::unique_ptr<FileSourceListener> listener_ = nullptr;
    std::atomic<bool> isReading_ = false;
    std::thread readThread_;
    std::unique_ptr<std::ifstream> fileIn_ = nullptr;
    std::shared_ptr<uint8_t> buffer_ = nullptr;
};
}
}
#endif