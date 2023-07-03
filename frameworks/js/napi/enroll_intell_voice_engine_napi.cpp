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

#include "enroll_intell_voice_engine_napi.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "intell_voice_napi_util.h"
#include "intell_voice_napi_queue.h"
#include "intell_voice_log.h"
#include "i_intell_voice_engine_callback.h"

#define LOG_TAG "EnrollEngineNapi"

using namespace OHOS::IntellVoice;
using namespace std;

namespace OHOS {
namespace IntellVoiceNapi {
const std::string ENROLL_ENGINE_NAPI_CLASS_NAME = "EnrollIntelligentVoiceEngine";

EnrollIntellVoiceEngineNapi::EnrollIntellVoiceEngineNapi()
{
    INTELL_VOICE_LOG_INFO("enter, %{public}p", this);
}

EnrollIntellVoiceEngineNapi::~EnrollIntellVoiceEngineNapi()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (wrapper_ != nullptr) {
        napi_delete_reference(env_, wrapper_);
    }
}

napi_value EnrollIntellVoiceEngineNapi::New(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t argc = 1;
    napi_value args[1] = {nullptr};

    napi_value jsThis = nullptr;
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr));

    unique_ptr<EnrollIntellVoiceEngineNapi> engineNapi = make_unique<EnrollIntellVoiceEngineNapi>();
    if (engineNapi == nullptr) {
        INTELL_VOICE_LOG_ERROR("Failed to create EnrollIntellVoiceEngineNapi, No memory.");
        return undefinedResult;
    }
    engineNapi->env_ = env;

    EnrollIntelligentVoiceEngineDescriptor descriptor = {};
    napi_value value = nullptr;
    if (napi_get_named_property(env, args[0], "wakeupPhrase", &value) == napi_ok) {
        GetValue(env, value, descriptor.wakeupPhrase);
    }
    INTELL_VOICE_LOG_INFO("EngineDescriptor wakeupPhrase: %{public}s", descriptor.wakeupPhrase.c_str());

    engineNapi->engine_ = std::make_shared<EnrollIntellVoiceEngine>(descriptor);
    if (engineNapi->engine_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("create intell voice engine faild.");
        return undefinedResult;
    }
    engineNapi->callbackNapi_ =  std::make_shared<EnrollIntellVoiceEngineCallbackNapi>(env);
    if (engineNapi->callbackNapi_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("create intell voice engine callback faild.");
        return undefinedResult;
    }
    engineNapi->engine_->SetCallback(engineNapi->callbackNapi_);

    auto finalize = [](napi_env env, void *data, void *hint) {
        INTELL_VOICE_LOG_INFO("EnrollIntellVoiceEngineNapi finalize callback");
        if (data != nullptr) {
            auto obj = static_cast<EnrollIntellVoiceEngineNapi *>(data);
            delete obj;
        }
    };

    NAPI_CALL(
        env, napi_wrap(env, jsThis, static_cast<void *>(engineNapi.get()), finalize, nullptr, &(engineNapi->wrapper_)));
    engineNapi.release();
    return jsThis;
}

napi_value EnrollIntellVoiceEngineNapi::Constructor(napi_env env)
{
    INTELL_VOICE_LOG_INFO("enter");
    napi_status status;
    napi_value constructor;
    napi_get_undefined(env, &constructor);

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getSupportedRegions", GetSupportedRegions),
        DECLARE_NAPI_FUNCTION("init", Init),
        DECLARE_NAPI_FUNCTION("start", Start),
        DECLARE_NAPI_FUNCTION("stop", Stop),
        DECLARE_NAPI_FUNCTION("setSensibility", SetSensibility),
        DECLARE_NAPI_FUNCTION("setWakeupHapInfo", SetWakeupHapInfo),
        DECLARE_NAPI_FUNCTION("setParameter", SetParameter),
        DECLARE_NAPI_FUNCTION("getParameter", GetParameter),
        DECLARE_NAPI_FUNCTION("release", Release),
        DECLARE_NAPI_FUNCTION("commit", Commit),
    };

    status = napi_define_class(env,
        ENROLL_ENGINE_NAPI_CLASS_NAME.c_str(),
        NAPI_AUTO_LENGTH,
        New,
        nullptr,
        sizeof(properties) / sizeof(properties[0]),
        properties,
        &constructor);

    return constructor;
}

napi_value EnrollIntellVoiceEngineNapi::GetSupportedRegions(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = 0;
    shared_ptr<AsyncContext> context = make_shared<AsyncContext>();
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create AsyncContext faild, No memory");
        return nullptr;
    }

    context->GetCbInfo(env, info, cbIndex, nullptr);

    AsyncExecute execute = [](napi_env env, void *data) {
        auto asyncContext = static_cast<AsyncContext *>(data);
        asyncContext->status_ = napi_ok;
    };

    AsyncComplete complete = [](AsyncContext *context, napi_value &result) {
        INTELL_VOICE_LOG_INFO("get supported regions complete");
        napi_create_array(context->env_, &result);
        napi_set_element(context->env_, result, 0, SetValue(context->env_, "CN"));
    };

    return NapiAsync::AsyncWork(context, "GetSupportedRegions", execute, complete);
}

napi_value EnrollIntellVoiceEngineNapi::Init(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = 1;
    auto context = make_shared<EnrollAsyncContext>();
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create InitContext faild, No memory");
        return nullptr;
    }

    CbInfoParser parser = [env, context](size_t argc, napi_value *argv) {
        napi_value temp = nullptr;
        if (napi_get_named_property(env, argv[0], "language", &temp) == napi_ok) {
            GetValue(env, temp,
                reinterpret_cast<EnrollIntellVoiceEngineNapi *>(context->engineNapi_)->config_.language);
        }
        if (napi_get_named_property(env, argv[0], "region", &temp) == napi_ok) {
            GetValue(env, temp,
                reinterpret_cast<EnrollIntellVoiceEngineNapi *>(context->engineNapi_)->config_.region);
        }
    };

    context->GetCbInfo(env, info, cbIndex, parser);

    context->type = ASYNC_WORK_INIT;
    context->callbackInfo.eventId = INTELLIGENT_VOICE_EVENT_ENROLL_INIT_DONE;
    context->callbackInfo.errCode = INTELLIGENT_VOICE_INIT_FAILED;
    context->processWork = [&](AsyncContext *asyncContext) {
        EnrollIntellVoiceEngineNapi *engineNapi = reinterpret_cast<EnrollIntellVoiceEngineNapi *>(
            asyncContext->engineNapi_);
        auto engine = engineNapi->engine_;
        if (engine == nullptr) {
            INTELL_VOICE_LOG_ERROR("get engine instance faild");
            return -1;
        }
        return engine->Init(engineNapi->config_);
    };

    AsyncExecute execute = [](napi_env env, void *data) {};

    return NapiAsync::AsyncWork(context, "Init", execute, CompleteCallback);
}

napi_value EnrollIntellVoiceEngineNapi::Start(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = 1;
    auto context = make_shared<EnrollAsyncContext>();
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create StartContext faild, No memory");
        return nullptr;
    }

    CbInfoParser parser = [env, context](size_t argc, napi_value *argv) {
        context->status_ = GetValue(env, argv[0],
            reinterpret_cast<EnrollIntellVoiceEngineNapi *>(context->engineNapi_)->isLast_);
        CHECK_STATUS_RETURN_VOID(context, "Failed to get isLast");
    };

    context->GetCbInfo(env, info, cbIndex, parser);

    context->type = ASYNC_WORK_START;
    context->callbackInfo.eventId = INTELLIGENT_VOICE_EVENT_ENROLL_COMPLETE;
    context->callbackInfo.errCode = INTELLIGENT_VOICE_ENROLL_FAILED;
    context->processWork = [&](AsyncContext *asyncContext) {
        EnrollIntellVoiceEngineNapi *engineNapi = reinterpret_cast<EnrollIntellVoiceEngineNapi *>(
            asyncContext->engineNapi_);
        auto engine = engineNapi->engine_;
        if (engine == nullptr) {
            INTELL_VOICE_LOG_ERROR("get engine instance faild");
            return -1;
        }

        return engine->Start(engineNapi->isLast_);
    };

    AsyncExecute execute = [](napi_env env, void *data) {};

    return NapiAsync::AsyncWork(context, "Start", execute, CompleteCallback);
}

napi_value EnrollIntellVoiceEngineNapi::Stop(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = 0;
    auto context = make_shared<AsyncContext>();
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create AsyncContext faild, No memory");
        return nullptr;
    }

    context->GetCbInfo(env, info, cbIndex, nullptr);

    auto cb = reinterpret_cast<EnrollIntellVoiceEngineNapi *>(context->engineNapi_)->callbackNapi_;
    if (cb != nullptr) {
        cb->ClearAsyncWork(false, "the requests was aborted because user called stop");
    }

    AsyncExecute execute = [](napi_env env, void *data) {
        auto asyncContext = static_cast<AsyncContext *>(data);
        auto engine = reinterpret_cast<EnrollIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_;
        if (engine == nullptr) {
            INTELL_VOICE_LOG_ERROR("get engine instance faild");
            return;
        }
        asyncContext->result_ = engine->Stop();
        asyncContext->status_ = napi_ok;
    };

    return NapiAsync::AsyncWork(context, "Stop", execute);
}

napi_value EnrollIntellVoiceEngineNapi::SetParameter(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = 2;
    class SetParameterContext : public AsyncContext {
    public:
        string key;
        string value;
    };
    auto context = make_shared<SetParameterContext>();
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create SetParameterContext faild, No memory");
        return nullptr;
    }

    CbInfoParser parser = [env, context](size_t argc, napi_value *argv) {
        context->status_ = GetValue(env, argv[0], context->key);
        CHECK_STATUS_RETURN_VOID(context, "Failed to get key");
        context->status_ = GetValue(env, argv[1], context->value);
        CHECK_STATUS_RETURN_VOID(context, "Failed to get value");
    };

    context->GetCbInfo(env, info, cbIndex, parser);

    AsyncExecute execute = [](napi_env env, void *data) {
        auto asyncContext = static_cast<SetParameterContext *>(data);
        auto engine = reinterpret_cast<EnrollIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_;
        if (engine == nullptr) {
            INTELL_VOICE_LOG_ERROR("get engine instance faild");
            return;
        }
        asyncContext->result_ = engine->SetParameter(asyncContext->key, asyncContext->value);
        asyncContext->status_ = napi_ok;
    };

    return NapiAsync::AsyncWork(context, "SetParameter", execute);
}

napi_value EnrollIntellVoiceEngineNapi::GetParameter(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = 1;
    shared_ptr<AsyncContext> context = make_shared<AsyncContext>();
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create AsyncContext faild, No memory");
        return nullptr;
    }

    context->GetCbInfo(env, info, cbIndex, nullptr);

    AsyncExecute execute = [](napi_env env, void *data) {
        auto asyncContext = static_cast<AsyncContext *>(data);
        asyncContext->status_ = napi_ok;
    };

    AsyncComplete complete = [](AsyncContext *context, napi_value &result) {
        INTELL_VOICE_LOG_INFO("get parameter complete");
        result = SetValue(context->env_, "value");
    };
    return NapiAsync::AsyncWork(context, "GetParameter", execute, complete);
}

napi_value EnrollIntellVoiceEngineNapi::SetSensibility(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = 1;
    class SetSensibilityContext : public AsyncContext {
    public:
        int32_t sensibility;
    };
    auto context = make_shared<SetSensibilityContext>();
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create SetSensibilityContext faild, No memory");
        return nullptr;
    }

    CbInfoParser parser = [env, context](size_t argc, napi_value *argv) {
        context->status_ = GetValue(env, argv[0], context->sensibility);
        CHECK_STATUS_RETURN_VOID(context, "Failed to get sensibility");
    };

    context->GetCbInfo(env, info, cbIndex, parser);

    AsyncExecute execute = [](napi_env env, void *data) {
        auto asyncContext = static_cast<SetSensibilityContext *>(data);
        auto engine = reinterpret_cast<EnrollIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_;
        if (engine == nullptr) {
            INTELL_VOICE_LOG_ERROR("get engine instance faild");
            return;
        }
        asyncContext->result_ = engine->SetSensibility(asyncContext->sensibility);
        asyncContext->status_ = napi_ok;
    };

    return NapiAsync::AsyncWork(context, "SetSensibility", execute);
}

napi_value EnrollIntellVoiceEngineNapi::SetWakeupHapInfo(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = 1;
    class SetWakeupHapContext : public AsyncContext {
    public:
        WakeupHapInfo hapInfo;
    };
    auto context = make_shared<SetWakeupHapContext>();
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create SetWakeupHapContext faild, No memory");
        return nullptr;
    }

    CbInfoParser parser = [env, context](size_t argc, napi_value *argv) {
        napi_value temp = nullptr;
        if (napi_get_named_property(env, argv[0], "bundleName", &temp) == napi_ok) {
            GetValue(env, temp, context->hapInfo.bundleName);
        }
        if (napi_get_named_property(env, argv[0], "abilityName", &temp) == napi_ok) {
            GetValue(env, temp, context->hapInfo.abilityName);
        }
    };

    context->GetCbInfo(env, info, cbIndex, parser);

    AsyncExecute execute = [](napi_env env, void *data) {
        auto asyncContext = static_cast<SetWakeupHapContext *>(data);
        auto engine = reinterpret_cast<EnrollIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_;
        if (engine == nullptr) {
            INTELL_VOICE_LOG_ERROR("get engine instance faild");
            return;
        }
        asyncContext->result_ = engine->SetWakeupHapInfo(asyncContext->hapInfo);
        asyncContext->status_ = napi_ok;
    };

    return NapiAsync::AsyncWork(context, "SetWakeupHapInfo", execute);
}

napi_value EnrollIntellVoiceEngineNapi::Commit(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = 0;
    auto context = make_shared<EnrollAsyncContext>();
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create IntellVoiceContext faild, No memory");
        return nullptr;
    }

    context->GetCbInfo(env, info, cbIndex, nullptr);

    context->type = ASYNC_WORK_COMMIT;
    context->callbackInfo.eventId = INTELLIGENT_VOICE_EVENT_COMMIT_ENROLL_COMPLETE;
    context->callbackInfo.errCode = INTELLIGENT_VOICE_COMMIT_ENROLL_FAILED;
    context->processWork = [](AsyncContext *asyncContext) {
        auto engine = reinterpret_cast<EnrollIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_;
        if (engine == nullptr) {
            INTELL_VOICE_LOG_ERROR("get engine instance faild");
            return -1;
        }

        return engine->Commit();
    };

    AsyncExecute execute = [](napi_env env, void *data) {};

    return NapiAsync::AsyncWork(context, "Commit", execute, CompleteCallback);
}

napi_value EnrollIntellVoiceEngineNapi::Release(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = 0;
    shared_ptr<AsyncContext> context = make_shared<AsyncContext>();
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create AsyncContext faild, No memory");
        return nullptr;
    }

    context->GetCbInfo(env, info, cbIndex, nullptr);

    AsyncExecute execute = [](napi_env env, void *data) {
        auto asyncContext = static_cast<AsyncContext *>(data);
        auto cb = reinterpret_cast<EnrollIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->callbackNapi_;
        if (cb != nullptr) {
            cb->ClearAsyncWork(false, "the requests was aborted because user called release");
        }
        auto engine = reinterpret_cast<EnrollIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_;
        if (engine == nullptr) {
            INTELL_VOICE_LOG_ERROR("get engine instance faild");
            return;
        }
        asyncContext->result_ = engine->Release();
        asyncContext->status_ = napi_ok;
        reinterpret_cast<EnrollIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_ = nullptr;
        reinterpret_cast<EnrollIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->callbackNapi_ = nullptr;
    };

    AsyncComplete complete = [](AsyncContext *context, napi_value &result) {
        INTELL_VOICE_LOG_INFO("release engine complete");
    };
    return NapiAsync::AsyncWork(context, "Release", execute, complete);
}

void EnrollIntellVoiceEngineNapi::CompleteCallback(napi_env env, napi_status status, void *data)
{
    INTELL_VOICE_LOG_INFO("enter");
    CHECK_RETURN_VOID(data != nullptr, "async complete callback data is null");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    auto asyncContext = static_cast<EnrollAsyncContext *>(data);
    if ((status != napi_ok) && (asyncContext->status_ == napi_ok)) {
        asyncContext->status_ = status;
        NapiAsync::CommonCallbackRoutine(asyncContext, result);
        return;
    }

    if (asyncContext->processWork == nullptr) {
        INTELL_VOICE_LOG_ERROR("process work is nullptr");
        NapiAsync::CommonCallbackRoutine(asyncContext, result);
        return;
    }

    auto cb = reinterpret_cast<EnrollIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->callbackNapi_;
    if (cb == nullptr) {
        INTELL_VOICE_LOG_ERROR("get callback napi faild");
        NapiAsync::CommonCallbackRoutine(asyncContext, result);
        return;
    }

    cb->QueueAsyncWork(asyncContext);
    if (asyncContext->processWork(asyncContext) != 0) {
        INTELL_VOICE_LOG_ERROR("process work failed");
        cb->ClearAsyncWork(true, "the request was aborted because intelligent voice processWork error");
    }
}
}  // namespace IntellVoiceNapi
}  // namespace OHOS