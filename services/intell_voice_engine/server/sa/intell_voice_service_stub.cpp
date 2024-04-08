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

#include "intell_voice_service_stub.h"
#include "intell_voice_log.h"

#define LOG_TAG "IntellVoiceServiceStub"

namespace OHOS {
namespace IntellVoiceEngine {
IntellVoiceServiceStub::IntellVoiceServiceStub()
{
    processServiceFuncMap_[HDI_INTELL_VOICE_SERVICE_CREATE_ENGINE] = [this](MessageParcel &data,
        MessageParcel &reply) -> int32_t { return this->CreateEngineInner(data, reply); };
    processServiceFuncMap_[HDI_INTELL_VOICE_SERVICE_RELEASE_ENGINE] = [this](MessageParcel &data,
        MessageParcel &reply) -> int32_t { return this->ReleaseEngineInner(data, reply); };
    processServiceFuncMap_[HDI_INTELL_VOICE_SERVICE_GET_PARAMETER] = [this](MessageParcel &data,
        MessageParcel &reply) -> int32_t { return this->GetParameterInner(data, reply); };
    processServiceFuncMap_[HDI_INTELL_VOICE_SERVICE_GET_REPORTED_FILES] = [this](MessageParcel &data,
        MessageParcel &reply) -> int32_t { return this->GetReportedFilesInner(data, reply); };
    processServiceFuncMap_[HDI_INTELL_VOICE_SERVICE_GET_CLONE_FILES_LIST] = [this](MessageParcel &data,
        MessageParcel &reply) -> int32_t { return this->GetCloneFileListInner(data, reply); };
    processServiceFuncMap_[HDI_INTELL_VOICE_SERVICE_GET_CLONE_FILE] = [this](MessageParcel &data,
        MessageParcel &reply) -> int32_t { return this->GetCloneFileInner(data, reply); };
    processServiceFuncMap_[HDI_INTELL_VOICE_SERVICE_SEND_CLONE_FILE] = [this](MessageParcel &data,
        MessageParcel &reply) -> int32_t { return this->SendCloneFileInner(data, reply); };
    processServiceFuncMap_[HDI_INTELL_VOICE_SERVICE_CLONE_FOR_RESULT] = [this](MessageParcel &data,
        MessageParcel &reply) -> int32_t { return this->CloneForResultInner(data, reply); };
}

IntellVoiceServiceStub::~IntellVoiceServiceStub()
{
    processServiceFuncMap_.clear();
}

int32_t IntellVoiceServiceStub::CreateEngineInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = 0;
    sptr<IIntellVoiceEngine> engine = nullptr;

    IntellVoiceEngineType type = static_cast<IntellVoiceEngineType>(data.ReadInt32());
    RegisterDeathRecipient(type, data.ReadRemoteObject());
    ret = CreateIntellVoiceEngine(type, engine);
    reply.WriteInt32(ret);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("failed to create engine, type:%{public}d", type);
        return ret;
    }
    if (engine != nullptr) {
        reply.WriteRemoteObject(engine->AsObject());
    }

    return ret;
}

int32_t IntellVoiceServiceStub::ReleaseEngineInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = 0;

    IntellVoiceEngineType type = static_cast<IntellVoiceEngineType>(data.ReadInt32());
    ret = ReleaseIntellVoiceEngine(type);
    reply.WriteInt32(ret);
    DeregisterDeathRecipient(type);
    return ret;
}

int32_t IntellVoiceServiceStub::GetReportedFilesInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = 0;
    int numMax =  data.ReadInt32();
    std::vector<UploadHdiFile> Files;
    ret = GetUploadFiles(numMax, Files);
    reply.WriteInt32(ret);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("get upload files failed, ret:%{public}d", ret);
        return ret;
    }
    int size = static_cast<int>(Files.size());
    reply.WriteInt32(size);
    INTELL_VOICE_LOG_INFO("reported hdi files size:%{public}d", size);
    if (size <= 0) {
        return -1;
    }
    for (auto file : Files) {
        reply.WriteInt32(file.type);
        reply.WriteString(file.filesDescription);
        reply.WriteInt32(file.filesContent.size());
        for (auto ashmem : file.filesContent) {
            reply.WriteAshmem(ashmem);
        }
    }
    return ret;
}

int32_t IntellVoiceServiceStub::GetParameterInner(MessageParcel &data, MessageParcel &reply)
{
    std::string val = GetParameter(data.ReadString());
    reply.WriteString(val);

    return 0;
}

int32_t IntellVoiceServiceStub::GetCloneFileListInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = 0;
    std::vector<std::string> cloneFiles;
    ret = GetWakeupSourceFilesList(cloneFiles);
    reply.WriteInt32(ret);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("get clone files list failed, ret:%{public}d", ret);
        return ret;
    }

    reply.WriteUint32(cloneFiles.size());
    for (auto i : cloneFiles) {
        reply.WriteString(i);
    }

    return ret;
}

int32_t IntellVoiceServiceStub::GetCloneFileInner(MessageParcel &data, MessageParcel &reply)
{
    std::string filePath = data.ReadString();
    std::vector<uint8_t> buffer;
    int32_t ret = GetWakeupSourceFile(filePath, buffer);
    reply.WriteInt32(ret);
    if (ret != 0) {
        INTELL_VOICE_LOG_ERROR("get clone file failed, ret:%{public}d", ret);
        return ret;
    }

    reply.WriteUint32(buffer.size());
    reply.WriteBuffer(buffer.data(), buffer.size());
    return ret;
}

int32_t IntellVoiceServiceStub::SendCloneFileInner(MessageParcel &data, MessageParcel &reply)
{
    std::string filePath = data.ReadString();
    std::vector<uint8_t> buffer;
    uint32_t size = data.ReadUint32();
    if (size == 0) {
        INTELL_VOICE_LOG_ERROR("invalid size");
        return -1;
    }

    const uint8_t *readBuf = data.ReadBuffer(size);
    if (readBuf == nullptr) {
        INTELL_VOICE_LOG_ERROR("read buffer is nullptr");
        return -1;
    }

    buffer.resize(size);
    std::copy(readBuf, readBuf + size, buffer.data());
    int32_t ret = SendWakeupFile(filePath, buffer);
    reply.WriteInt32(ret);
    return ret;
}

int32_t IntellVoiceServiceStub::CloneForResultInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = 0;
    std::string wakeupInfo = data.ReadString();
    sptr<IRemoteObject> object = data.ReadRemoteObject();

    ret = EnrollWithWakeupFilesForResult(wakeupInfo, object);
    reply.WriteInt32(ret);
    return ret;
}

int32_t IntellVoiceServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IIntellVoiceService::GetDescriptor()) {
        INTELL_VOICE_LOG_ERROR("token mismatch");
        return -1;
    }

    auto it = processServiceFuncMap_.find(code);
    if ((it != processServiceFuncMap_.end()) && (it->second != nullptr)) {
        return it->second(data, reply);
    }

    INTELL_VOICE_LOG_WARN("sevice stub invalid code:%{public}u", code);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace IntellVoiceEngine
}  // namespace OHOS
