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

#include "intell_voice_common_napi.h"
#include "intell_voice_napi_util.h"
#include "intell_voice_napi_queue.h"

#include "intell_voice_log.h"
#include "i_intell_voice_engine_callback.h"

#define LOG_TAG "WakeupIntellVoiceEngineNapi"

using namespace std;
using namespace OHOS::IntellVoice;

namespace OHOS {
namespace IntellVoiceNapi {
static __thread napi_ref g_wakeupEngineConstructor = nullptr;
WakeupIntelligentVoiceEngineDescriptor WakeupIntellVoiceEngineNapi::g_wakeupEngineDesc_;
int32_t WakeupIntellVoiceEngineNapi::constructResult_ = NAPI_INTELLIGENT_VOICE_SUCCESS;

static const std::string WAKEUP_ENGINE_NAPI_CLASS_NAME = "WakeupIntelligentVoiceEngine";
static const std::string INTELL_VOICE_EVENT_CALLBACK_NAME = "wakeupIntelligentVoiceEvent";

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

napi_value WakeupIntellVoiceEngineNapi::Export(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_get_undefined(env, &result);

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

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION(
            "createWakeupIntelligentVoiceEngine", CreateWakeupIntelligentVoiceEngine)
    };

    status = napi_define_class(env,
        WAKEUP_ENGINE_NAPI_CLASS_NAME.c_str(),
        NAPI_AUTO_LENGTH,
        Construct,
        nullptr,
        sizeof(properties) / sizeof(properties[0]),
        properties,
        &constructor);
    if (status != napi_ok) {
        return result;
    }

    status = napi_create_reference(env, constructor, refCount, &g_wakeupEngineConstructor);
    if (status == napi_ok) {
        status = napi_set_named_property(env, exports, WAKEUP_ENGINE_NAPI_CLASS_NAME.c_str(), constructor);
        if (status == napi_ok) {
            status = napi_define_properties(env, exports,
                                            sizeof(static_prop) / sizeof(static_prop[0]), static_prop);
            if (status == napi_ok) {
                return exports;
            }
        }
    }

    return result;
}

void WakeupIntellVoiceEngineNapi::Destruct(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<WakeupIntellVoiceEngineNapi *>(nativeObject);
        delete obj;
    }
}

napi_value WakeupIntellVoiceEngineNapi::Construct(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    napi_status status;
    size_t argc = ARGC_ONE;
    napi_value args[ARGC_ONE] = { nullptr };

    napi_value jsThis = nullptr;
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    constructResult_ = NAPI_INTELLIGENT_VOICE_SUCCESS;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr));

    unique_ptr<WakeupIntellVoiceEngineNapi> engineNapi = make_unique<WakeupIntellVoiceEngineNapi>();
    if (engineNapi == nullptr) {
        INTELL_VOICE_LOG_ERROR("Failed to create WakeupIntellVoiceEngineNapi, No memory");
        constructResult_ = NAPI_INTELLIGENT_VOICE_NO_MEMORY;
        return undefinedResult;
    }

    engineNapi->env_ = env;
    engineNapi->engine_ = std::make_shared<WakeupIntellVoiceEngine>(g_wakeupEngineDesc_);
    if (engineNapi->engine_ == nullptr) {
        INTELL_VOICE_LOG_ERROR("create intell voice engine failed");
        constructResult_ = NAPI_INTELLIGENT_VOICE_NO_MEMORY;
        return undefinedResult;
    }

    if (engineNapi->engine_->GetEngine() == nullptr) {
        INTELL_VOICE_LOG_ERROR("create wakeup engine failed");
        constructResult_ = NAPI_INTELLIGENT_VOICE_PERMISSION_DENIED;
        return undefinedResult;
    }

    status = napi_wrap(env, jsThis, static_cast<void *>(engineNapi.get()), WakeupIntellVoiceEngineNapi::Destruct,
        nullptr, &(engineNapi->wrapper_));
    if (status == napi_ok) {
        engineNapi.release();
        return jsThis;
    }

    INTELL_VOICE_LOG_ERROR("wrap wakeup intell voice engine failed");
    return undefinedResult;
}

napi_value WakeupIntellVoiceEngineNapi::CreateWakeupIntelligentVoiceEngine(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = ARG_INDEX_1;
    auto context = std::make_shared<AsyncContext>(env);
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create context failed, No memory");
        return nullptr;
    }

    CbInfoParser parser = [env, context](size_t argc, napi_value *argv) -> bool {
        CHECK_CONDITION_RETURN_FALSE((argc < ARGC_ONE), "argc less than 1");
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        CHECK_CONDITION_RETURN_FALSE((valueType != napi_object), "arg type mismatch");
        napi_value temp = nullptr;
        napi_status status = napi_get_named_property(env, argv[0], "needReconfirm", &temp);
        CHECK_CONDITION_RETURN_FALSE((status != napi_ok), "get property failed");
        status = GetValue(env, temp, g_wakeupEngineDesc_.needReconfirm);
        CHECK_CONDITION_RETURN_FALSE((status != napi_ok), "get need reconfirm failed");
        status = napi_get_named_property(env, argv[0], "wakeupPhrase", &temp);
        CHECK_CONDITION_RETURN_FALSE((status != napi_ok), "get property failed");
        status = GetValue(env, temp, g_wakeupEngineDesc_.wakeupPhrase);
        CHECK_CONDITION_RETURN_FALSE((status != napi_ok), "get wakeup phrase failed");
        INTELL_VOICE_LOG_INFO("needReconfirm: %{public}d", g_wakeupEngineDesc_.needReconfirm);
        INTELL_VOICE_LOG_INFO("wakeupPhrase: %{public}s", g_wakeupEngineDesc_.wakeupPhrase.c_str());
        return true;
    };

    context->result_ = (context->GetCbInfo(env, info, cbIndex, parser) ? NAPI_INTELLIGENT_VOICE_SUCCESS :
        NAPI_INTELLIGENT_VOICE_INVALID_PARAM);

    AsyncExecute execute = [](napi_env env, void *data) {};

    context->complete_ = [](napi_env env, AsyncContext *asyncContext, napi_value &result) {
        napi_value constructor = nullptr;
        napi_status status = napi_get_reference_value(env, g_wakeupEngineConstructor, &constructor);
        if (status != napi_ok) {
            asyncContext->result_ = NAPI_INTELLIGENT_VOICE_NO_MEMORY;
            napi_get_undefined(env, &result);
            return;
        }

        status = napi_new_instance(env, constructor, 0, nullptr, &result);
        asyncContext->result_ = constructResult_;
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        }
    };

    return NapiAsync::AsyncWork(env, context, "CreateWakeupIntelligentVoiceEngine", execute);
}

napi_value WakeupIntellVoiceEngineNapi::GetSupportedRegions(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = ARG_INDEX_0;
    shared_ptr<AsyncContext> context = make_shared<AsyncContext>(env);
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create AsyncContext failed, No memory");
        return nullptr;
    }

    context->result_ = (context->GetCbInfo(env, info, cbIndex, nullptr) ? NAPI_INTELLIGENT_VOICE_SUCCESS :
        NAPI_INTELLIGENT_VOICE_INVALID_PARAM);

    AsyncExecute execute = [](napi_env env, void *data) {};

    context->complete_ = [](napi_env env, AsyncContext *asyncContext, napi_value &result) {
        napi_create_array(env, &result);
        napi_set_element(env, result, 0, SetValue(env, "CN"));
    };

    return NapiAsync::AsyncWork(env, context, "GetSupportedRegions", execute);
}

napi_value WakeupIntellVoiceEngineNapi::SetParameter(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = ARG_INDEX_2;
    class SetParameterContext : public AsyncContext {
    public:
        explicit SetParameterContext(napi_env napiEnv) : AsyncContext(napiEnv) {};
        string key;
        string value;
    };
    auto context = make_shared<SetParameterContext>(env);
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create SetParameterContext failed, No memory");
        return nullptr;
    }

    CbInfoParser parser = [env, context](size_t argc, napi_value *argv) -> bool {
        CHECK_CONDITION_RETURN_FALSE((argc < ARGC_TWO), "argc less than 2");
        CHECK_CONDITION_RETURN_FALSE((GetValue(env, argv[ARG_INDEX_0], context->key) != napi_ok), "Failed to get key");
        CHECK_CONDITION_RETURN_FALSE((GetValue(env, argv[ARG_INDEX_1], context->value) != napi_ok),
            "Failed to get value");
        return true;
    };

    context->result_ = (context->GetCbInfo(env, info, cbIndex, parser) ? NAPI_INTELLIGENT_VOICE_SUCCESS :
        NAPI_INTELLIGENT_VOICE_INVALID_PARAM);

    AsyncExecute execute;
    if (context->result_ == NAPI_INTELLIGENT_VOICE_SUCCESS) {
        execute = [](napi_env env, void *data) {
            CHECK_CONDITION_RETURN_VOID((data == nullptr), "data is nullptr");
            auto asyncContext = static_cast<SetParameterContext *>(data);
            auto engine = reinterpret_cast<WakeupIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_;
            CHECK_CONDITION_RETURN_VOID((engine == nullptr), "get engine instance failed");
            engine->SetParameter(asyncContext->key, asyncContext->value);
        };
    } else {
        execute = [](napi_env env, void *data) {};
    }
    return NapiAsync::AsyncWork(env, context, "SetParameter", execute);
}

napi_value WakeupIntellVoiceEngineNapi::GetParameter(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = ARG_INDEX_1;
    shared_ptr<AsyncContext> context = make_shared<AsyncContext>(env);
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create AsyncContext failed, No memory");
        return nullptr;
    }

    context->result_ = (context->GetCbInfo(env, info, cbIndex, nullptr) ? NAPI_INTELLIGENT_VOICE_SUCCESS :
        NAPI_INTELLIGENT_VOICE_INVALID_PARAM);

    AsyncExecute execute = [](napi_env env, void *data) {};

    context->complete_ = [](napi_env env, AsyncContext *asyncContext, napi_value &result) {
        INTELL_VOICE_LOG_INFO("get parameter enter");
        result = SetValue(env, "value");
    };

    return NapiAsync::AsyncWork(env, context, "GetParameter", execute);
}

napi_value WakeupIntellVoiceEngineNapi::SetSensibility(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = ARG_INDEX_1;
    class SetSensibilityContext : public AsyncContext {
    public:
        explicit SetSensibilityContext(napi_env napiEnv) : AsyncContext(napiEnv) {};
        int32_t sensibility;
    };
    auto context = make_shared<SetSensibilityContext>(env);
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create SetSensibilityContext failed, No memory");
        return nullptr;
    }

    CbInfoParser parser = [env, context](size_t argc, napi_value *argv) -> bool {
        CHECK_CONDITION_RETURN_FALSE((argc < ARGC_ONE), "argc less than 1");
        CHECK_CONDITION_RETURN_FALSE((GetValue(env, argv[0], context->sensibility) != napi_ok),
            "Failed to get sensibility");
        return true;
    };

    context->result_ = (context->GetCbInfo(env, info, cbIndex, parser) ? NAPI_INTELLIGENT_VOICE_SUCCESS :
        NAPI_INTELLIGENT_VOICE_INVALID_PARAM);

    AsyncExecute execute = [](napi_env env, void *data) {
        CHECK_CONDITION_RETURN_VOID((data == nullptr), "data is nullptr");
        auto asyncContext = static_cast<SetSensibilityContext *>(data);
        CHECK_CONDITION_RETURN_VOID((asyncContext->result_ != NAPI_INTELLIGENT_VOICE_SUCCESS), "no need to execute");
        auto engine = reinterpret_cast<WakeupIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_;
        CHECK_CONDITION_RETURN_VOID((engine == nullptr), "get engine instance failed");
        engine->SetSensibility(asyncContext->sensibility);
    };

    return NapiAsync::AsyncWork(env, context, "SetSensibility", execute);
}

napi_value WakeupIntellVoiceEngineNapi::SetWakeupHapInfo(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = ARG_INDEX_1;
    class SetWakeupHapContext : public AsyncContext {
    public:
        explicit SetWakeupHapContext(napi_env napiEnv) : AsyncContext(napiEnv) {};
        WakeupHapInfo hapInfo;
    };
    auto context = make_shared<SetWakeupHapContext>(env);
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create SetWakeupHapContext failed, No memory");
        return nullptr;
    }

    CbInfoParser parser = [env, context](size_t argc, napi_value *argv) -> bool {
        CHECK_CONDITION_RETURN_FALSE((argc < ARGC_ONE), "argc less than 1");
        napi_value temp = nullptr;
        CHECK_CONDITION_RETURN_FALSE((napi_get_named_property(env, argv[0], "bundleName", &temp) != napi_ok),
            "get property failed");
        CHECK_CONDITION_RETURN_FALSE((GetValue(env, temp, context->hapInfo.bundleName) != napi_ok),
            "get bundle name failed");
        CHECK_CONDITION_RETURN_FALSE((napi_get_named_property(env, argv[0], "abilityName", &temp) != napi_ok),
            "get property failed");
        CHECK_CONDITION_RETURN_FALSE((GetValue(env, temp, context->hapInfo.abilityName) != napi_ok),
            "get ability name failed");
        return true;
    };

    context->result_ = (context->GetCbInfo(env, info, cbIndex, parser) ? NAPI_INTELLIGENT_VOICE_SUCCESS :
        NAPI_INTELLIGENT_VOICE_INVALID_PARAM);

    AsyncExecute execute = [](napi_env env, void *data) {
        CHECK_CONDITION_RETURN_VOID((data == nullptr), "data is nullptr");
        auto asyncContext = static_cast<SetWakeupHapContext *>(data);
        CHECK_CONDITION_RETURN_VOID((asyncContext->result_ != NAPI_INTELLIGENT_VOICE_SUCCESS), "no need to execute");
        auto engine = reinterpret_cast<WakeupIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_;
        CHECK_CONDITION_RETURN_VOID((engine == nullptr), "get engine instance failed");
        engine->SetWakeupHapInfo(asyncContext->hapInfo);
    };

    return NapiAsync::AsyncWork(env, context, "SetWakeupHapInfo", execute);
}

napi_value WakeupIntellVoiceEngineNapi::Release(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");
    size_t cbIndex = ARG_INDEX_0;
    shared_ptr<AsyncContext> context = make_shared<AsyncContext>(env);
    if (context == nullptr) {
        INTELL_VOICE_LOG_ERROR("create AsyncContext failed, No memory");
        return nullptr;
    }

    context->result_ = (context->GetCbInfo(env, info, cbIndex, nullptr) ? NAPI_INTELLIGENT_VOICE_SUCCESS :
        NAPI_INTELLIGENT_VOICE_INVALID_PARAM);

    AsyncExecute execute = [](napi_env env, void *data) {
        CHECK_CONDITION_RETURN_VOID((data == nullptr), "data is nullptr");
        auto asyncContext = static_cast<AsyncContext *>(data);
        CHECK_CONDITION_RETURN_VOID((asyncContext->result_ != NAPI_INTELLIGENT_VOICE_SUCCESS), "no need to execute");
        auto engine = reinterpret_cast<WakeupIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_;
        CHECK_CONDITION_RETURN_VOID((engine == nullptr), "get engine instance failed");
        engine->Release();
        reinterpret_cast<WakeupIntellVoiceEngineNapi *>(asyncContext->engineNapi_)->engine_ = nullptr;
    };

    return NapiAsync::AsyncWork(env, context, "Release", execute);
}

napi_value WakeupIntellVoiceEngineNapi::On(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");

    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    size_t argCount = ARGC_TWO;
    napi_value args[ARGC_TWO] = {0};
    napi_value jsThis = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    if (status != napi_ok || argCount != ARGC_TWO) {
        INTELL_VOICE_LOG_ERROR("failed to get parameters");
        return undefinedResult;
    }

    napi_valuetype eventType = napi_undefined;
    if (napi_typeof(env, args[ARG_INDEX_0], &eventType) != napi_ok || eventType != napi_string) {
        INTELL_VOICE_LOG_ERROR("callback event name type mismatch");
        return undefinedResult;
    }

    string callbackName = "";
    status = GetValue(env, args[ARG_INDEX_0], callbackName);
    if (status != napi_ok) {
        INTELL_VOICE_LOG_ERROR("failed to get callbackName");
        return undefinedResult;
    }
    INTELL_VOICE_LOG_INFO("callbackName: %{public}s", callbackName.c_str());

    napi_valuetype handler = napi_undefined;
    if (napi_typeof(env, args[ARG_INDEX_1], &handler) != napi_ok || handler != napi_function) {
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

    engineNapi->callbackNapi_ = make_shared<EngineEventCallbackNapi>(env, args[ARG_INDEX_1]);
    engineNapi->engine_->SetCallback(engineNapi->callbackNapi_);
    INTELL_VOICE_LOG_INFO("Set callback finish");
    return result;
}

napi_value WakeupIntellVoiceEngineNapi::Off(napi_env env, napi_callback_info info)
{
    INTELL_VOICE_LOG_INFO("enter");

    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    size_t argCount = ARGC_ONE;
    napi_value args[ARGC_ONE] = { nullptr };
    napi_value jsThis = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    if (status != napi_ok || argCount != ARGC_ONE) {
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
    engineNapi->callbackNapi_ = nullptr;

    return result;
}
}  // namespace IntellVoiceNapi
}  // namespace OHOS
