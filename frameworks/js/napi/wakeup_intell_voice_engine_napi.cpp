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

#include "wakeup_intell_voice_engine_napi.h"

#include "want.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "intell_voice_napi_util.h"
#include "intell_voice_napi_queue.h"

#include "intell_voice_log.h"
#include "i_intell_voice_engine_callback.h"

#define LOG_TAG "WakeupIntellVoiceEngineNapi"

using namespace std;
using namespace OHOS::IntellVoice;

namespace OHOS {
namespace IntellVoiceNapi {
static const std::string INTELL_VOICE_EVENT_CALLBACK_NAME = "IntellVoiceEvent";
const std::string WAKEUP_ENGINE_NAPI_CLASS_NAME = "WakeupIntelligentVoiceEngine";

WakeupIntellVoiceEngineNapi::WakeupIntellVoiceEngineNapi()
{
    INTELL_VOICE_LOG_INFO("enter, %{public}p", this);
}

WakeupIntellVoiceEngineNapi::~WakeupIntellVoiceEngineNapi()
{
    INTELL_VOICE_LOG_INFO("enter");
    if (wrapper_ != nullptr) {
        napi_delete_reference(env_, wrapper_);
    }
}

napi_value WakeupIntellVoiceEngineNapi::New(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t argc = 1;
    napi_value args[1] = {nullptr};

    napi_value jsThis = nullptr;
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr));

    unique_ptr<WakeupIntellVoiceEngineNapi> engineNapi = make_unique<WakeupIntellVoiceEngineNapi>();
    if (engineNapi == nullptr) {
        INTELL_VOICE_LOG_ERROR("Failed to create WakeupIntellVoiceEngineNapi, No memory.");
        return undefinedResult;
    }
    engineNapi->env_ = env;

    WakeupIntelligentVoiceEngineDescriptor descriptor = {};
    napi_value value = nullptr;
    if (napi_get_named_property(env, args[0], "needApAlgEngine", &value) == napi_ok) {
        GetValue(env, value, descriptor.needApAlgEngine);
    }
    if (napi_get_named_property(env, args[0], "wakeupPhrase", &value) == napi_ok) {
        GetValue(env, value, descriptor.wakeupPhrase);
    }
    INTELL_VOICE_LOG_INFO("EngineDescriptor needApAlgEngine: %{public}d", descriptor.needApAlgEngine);
    INTELL_VOICE_LOG_INFO("EngineDescriptor wakeupPhrase: %{public}s", descriptor.wakeupPhrase.c_str());

    engineNapi->engine_ = std::make_shared<WakeupIntellVoiceEngine>(descriptor);
    if (engineNapi->engine_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("create intell voice engine faild.");
        return undefinedResult;
    }

    auto finalize = [](napi_env env, void *data, void *hint) {
        INTELL_VOICE_LOG_INFO("WakeupIntellVoiceEngineNapi finalize callback");
        if (data != nullptr) {
            auto obj = static_cast<WakeupIntellVoiceEngineNapi *>(data);
            delete obj;
        }
    };

    NAPI_CALL(env,
        napi_wrap(env, jsThis, static_cast<void *>(engineNapi.get()), finalize, nullptr, &(engineNapi->wrapper_)));
    engineNapi.release();
    return jsThis;
}

napi_value WakeupIntellVoiceEngineNapi::Constructor(napi_env env)
{
    INTELL_VOICE_LOG_INFO("enter");
    napi_status status;
    napi_value constructor;
    napi_get_undefined(env, &constructor);

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getSupportedRegions", GetSupportedRegions),
        DECLARE_NAPI_FUNCTION("setSensibility", SetSensibility),
        DECLARE_NAPI_FUNCTION("setWakeupHapInfo", SetWakeupHapInfo),
        DECLARE_NAPI_FUNCTION("setParameter", SetParameter),
        DECLARE_NAPI_FUNCTION("getParameter", GetParameter),
        DECLARE_NAPI_FUNCTION("release", Release),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
    };

    status = napi_define_class(env,
        WAKEUP_ENGINE_NAPI_CLASS_NAME.c_str(),
        NAPI_AUTO_LENGTH,
        New,
        nullptr,
        sizeof(properties) / sizeof(properties[0]),
        properties,
        &constructor);

    return constructor;
}

napi_value WakeupIntellVoiceEngineNapi::GetSupportedRegions(napi_env env, napi_callback_info info)
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

napi_value WakeupIntellVoiceEngineNapi::SetParameter(napi_env env, napi_callback_info info)
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
        auto engine = reinterpret_cast<WakeupIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_;
        if (engine == nullptr) {
            INTELL_VOICE_LOG_ERROR("get engine instance faild");
            return;
        }
        asyncContext->result_ = engine->SetParameter(asyncContext->key, asyncContext->value);
        asyncContext->status_ = napi_ok;
    };

    return NapiAsync::AsyncWork(context, "SetParameter", execute);
}

napi_value WakeupIntellVoiceEngineNapi::GetParameter(napi_env env, napi_callback_info info)
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

napi_value WakeupIntellVoiceEngineNapi::SetSensibility(napi_env env, napi_callback_info info)
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
        auto engine = reinterpret_cast<WakeupIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_;
        if (engine == nullptr) {
            INTELL_VOICE_LOG_ERROR("get engine instance faild");
            return;
        }
        asyncContext->result_ = engine->SetSensibility(asyncContext->sensibility);
        asyncContext->status_ = napi_ok;
    };

    return NapiAsync::AsyncWork(context, "SetSensibility", execute);
}

napi_value WakeupIntellVoiceEngineNapi::SetWakeupHapInfo(napi_env env, napi_callback_info info)
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
        auto engine = reinterpret_cast<WakeupIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_;
        if (engine == nullptr) {
            INTELL_VOICE_LOG_ERROR("get engine instance faild");
            return;
        }
        asyncContext->result_ = engine->SetWakeupHapInfo(asyncContext->hapInfo);
        asyncContext->status_ = napi_ok;
    };

    return NapiAsync::AsyncWork(context, "SetWakeupHapInfo", execute);
}

napi_value WakeupIntellVoiceEngineNapi::Release(napi_env env, napi_callback_info info)
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
        auto engine = reinterpret_cast<WakeupIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_;
        if (engine == nullptr) {
            INTELL_VOICE_LOG_ERROR("get engine instance faild");
            return;
        }
        asyncContext->result_ = engine->Release();
        asyncContext->status_ = napi_ok;
        reinterpret_cast<WakeupIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_ = nullptr;
    };

    AsyncComplete complete = [](AsyncContext *context, napi_value &result) {
        INTELL_VOICE_LOG_INFO("release engine complete");
    };
    return NapiAsync::AsyncWork(context, "Release", execute, complete);
}

napi_value WakeupIntellVoiceEngineNapi::On(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");

    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    const size_t expectArgCount = 2;
    size_t argCount = 2;
    napi_value args[expectArgCount] = {0};
    napi_value jsThis = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    if (status != napi_ok || argCount != expectArgCount) {
        INTELL_VOICE_LOG_ERROR("failed to get parameters");
        return undefinedResult;
    }

    napi_valuetype eventType = napi_undefined;
    if (napi_typeof(env, args[0], &eventType) != napi_ok || eventType != napi_string) {
        INTELL_VOICE_LOG_ERROR("callback event name type mismatch");
        return undefinedResult;
    }

    string callbackName = "";
    status = GetValue(env, args[0], callbackName);
    if (status != napi_ok) {
        INTELL_VOICE_LOG_ERROR("failed to get callbackName");
        return undefinedResult;
    }
    INTELL_VOICE_LOG_INFO("callbackName: %{public}s", callbackName.c_str());

    napi_valuetype handler = napi_undefined;
    if (napi_typeof(env, args[1], &handler) != napi_ok || handler != napi_function) {
        INTELL_VOICE_LOG_ERROR("callback handler type mismatch");
        return undefinedResult;
    }

    return RegisterCallback(env, jsThis, args);
}

napi_value WakeupIntellVoiceEngineNapi::RegisterCallback(napi_env env, napi_value jsThis, napi_value *args)
{
    INTELL_VOICE_LOG_INFO("enter");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    WakeupIntellVoiceEngineNapi *engineNapi = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&engineNapi));
    if (status != napi_ok || engineNapi == nullptr) {
        INTELL_VOICE_LOG_ERROR("Failed to get engine napi instance");
        return result;
    }

    if (engineNapi->engine_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("engine is null");
        return result;
    }

    engineNapi->callbackNapi_ = make_shared<EngineEventCallbackNapi>(env, args[1]);
    engineNapi->engine_->SetCallback(engineNapi->callbackNapi_);
    INTELL_VOICE_LOG_INFO("Set callback finish");
    return result;
}

napi_value WakeupIntellVoiceEngineNapi::Off(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");

    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    const size_t expectArgCount = 1;
    size_t argCount = 1;
    napi_value args[expectArgCount] = {0};
    napi_value jsThis = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    if (status != napi_ok || argCount != expectArgCount) {
        INTELL_VOICE_LOG_ERROR("failed to get parameters");
        return undefinedResult;
    }

    napi_valuetype eventType = napi_undefined;
    if (napi_typeof(env, args[0], &eventType) != napi_ok || eventType != napi_string) {
        INTELL_VOICE_LOG_ERROR("callback event name type mismatch");
        return undefinedResult;
    }

    string callbackName = "";
    status = GetValue(env, args[0], callbackName);
    if (status != napi_ok) {
        INTELL_VOICE_LOG_ERROR("failed to get callbackName");
        return undefinedResult;
    }
    INTELL_VOICE_LOG_INFO("callbackName: %{public}s", callbackName.c_str());

    return UnregisterCallback(env, jsThis, callbackName);
}

napi_value WakeupIntellVoiceEngineNapi::UnregisterCallback(napi_env env, napi_value jsThis, const string &callbackName)
{
    INTELL_VOICE_LOG_INFO("enter");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    WakeupIntellVoiceEngineNapi *engineNapi = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&engineNapi));
    if (status != napi_ok || engineNapi == nullptr) {
        INTELL_VOICE_LOG_ERROR("Failed to get engine napi instance");
        return result;
    }

    if (callbackName != INTELL_VOICE_EVENT_CALLBACK_NAME) {
        INTELL_VOICE_LOG_ERROR("No such off callback supported");
        return result;
    }

    return result;
}
}  // namespace IntellVoiceNapi
}  // namespace OHOS