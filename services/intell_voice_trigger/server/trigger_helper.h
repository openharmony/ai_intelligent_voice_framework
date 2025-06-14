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
#ifndef INTELL_VOICE_TRIGGER_HELPER_H
#define INTELL_VOICE_TRIGGER_HELPER_H

#include <cstdint>
#include <map>
#include "msg_handle_thread.h"
#include "trigger_base_type.h"
#include "i_intell_voice_trigger_recognition_callback.h"
#include "i_intell_voice_trigger_connector_module.h"
#include "i_intell_voice_trigger_connector_callback.h"
#include "trigger_connector_common_type.h"
#ifdef SUPPORT_TELEPHONY_SERVICE
#include "telephony_observer.h"
#endif
#include "audio_system_manager.h"
#include "audio_stream_manager.h"
#include "audio_info.h"
#ifdef POWER_MANAGER_ENABLE
#include "sync_hibernate_callback_stub.h"
#include "sync_sleep_callback_stub.h"
#include "power_mgr_client.h"
#endif
#ifdef SUPPORT_WINDOW_MANAGER
#include "display_manager_lite.h"
#endif

namespace OHOS {
namespace IntellVoiceTrigger {
using OHOS::AudioStandard::AudioCapturerSourceCallback;
using OHOS::AudioStandard::AudioRendererStateChangeCallback;
using OHOS::AudioStandard::AudioRendererChangeInfo;
using OHOS::AudioStandard::AudioManagerAudioSceneChangedCallback;
using OHOS::AudioStandard::AudioScene;
#ifdef SUPPORT_WINDOW_MANAGER
using FoldStatus = OHOS::Rosen::FoldStatus;
#endif

enum ModelState { MODEL_NOTLOADED, MODEL_LOADED, MODEL_STARTED, MODEL_STATE_BUT };

class TriggerModelData {
public:
    explicit TriggerModelData(int32_t uuid);
    ~TriggerModelData();
    void SetCallback(std::shared_ptr<IIntellVoiceTriggerRecognitionCallback> callback);
    std::shared_ptr<IIntellVoiceTriggerRecognitionCallback> GetCallback();
    void SetModel(std::shared_ptr<GenericTriggerModel> model);
    std::shared_ptr<GenericTriggerModel> GetModel();
    void SetState(ModelState state);
    ModelState GetState() const;
    void SetModelHandle(int32_t handle);
    int32_t GetModelHandle() const;
    void SetRequested(bool requested);
    bool GetRequested() const;

    bool SameModel(std::shared_ptr<GenericTriggerModel> model);
    void Clear();
    void ClearCallback();

public:
    int32_t uuid_ = -1;

private:
    ModelState state_ = MODEL_NOTLOADED;
    std::shared_ptr<GenericTriggerModel> model_ = nullptr;
    std::shared_ptr<IIntellVoiceTriggerRecognitionCallback> callback_ = nullptr;
    int32_t modelHandle_ = 0;
    bool requested_ = false;
};

class TriggerHelper : public IIntellVoiceTriggerConnectorCallback, public std::enable_shared_from_this<TriggerHelper> {
public:
    ~TriggerHelper();

    static std::shared_ptr<TriggerHelper> Create();

    int32_t StartGenericRecognition(int32_t uuid, std::shared_ptr<GenericTriggerModel> model,
        std::shared_ptr<IIntellVoiceTriggerRecognitionCallback> callback);
    int32_t StopGenericRecognition(int32_t uuid, std::shared_ptr<IIntellVoiceTriggerRecognitionCallback> callback);
    void UnloadGenericTriggerModel(int32_t uuid);
    int32_t SetParameter(const std::string &key, const std::string &value);
    std::string GetParameter(const std::string &key);
    void AttachAudioCaptureListener();
    void DetachAudioCaptureListener();
    void AttachAudioRendererEventListener();
    void DetachAudioRendererEventListener();
    void AttachAudioSceneEventListener();
    void DetachAudioSceneEventListener();
#ifdef SUPPORT_TELEPHONY_SERVICE
    void AttachTelephonyObserver();
    void DetachTelephonyObserver();
#endif
#ifdef POWER_MANAGER_ENABLE
    void AttachHibernateObserver();
    void DetachHibernateObserver();
#endif

private:
    TriggerHelper();
    bool GetModule();
    std::shared_ptr<TriggerModelData> GetTriggerModelData(int32_t uuid);
    std::shared_ptr<TriggerModelData> CreateTriggerModelData(int32_t uuid);
    int32_t InitRecognition(std::shared_ptr<TriggerModelData> modelData, bool unload);
    int32_t PrepareForRecognition(std::shared_ptr<TriggerModelData> modelData);
    int32_t StartRecognition(std::shared_ptr<TriggerModelData> modelData);
    int32_t StopRecognition(std::shared_ptr<TriggerModelData> modelData);
    int32_t LoadModel(std::shared_ptr<TriggerModelData> modelData);
    int32_t UnloadModel(std::shared_ptr<TriggerModelData> modelData);
    void OnUpdateAllRecognitionState();
    bool IsConflictSceneActive();

    void OnRecognition(int32_t modelHandle, const IntellVoiceRecognitionEvent &event) override;
    void OnCapturerStateChange(bool isActive);
    void OnUpdateRendererState(int32_t streamUsage, bool isPlaying);
#ifdef POWER_MANAGER_ENABLE
    void OnHibernateStateUpdated(bool isHibernate);
#endif
    void OnAudioSceneChange(const AudioScene audioScene);
#ifdef SUPPORT_TELEPHONY_SERVICE
    void OnCallStateUpdated(int32_t callState);
    class TelephonyStateObserver : public Telephony::TelephonyObserver {
    public:
        explicit TelephonyStateObserver(const std::shared_ptr<TriggerHelper> helper) : helper_(helper)
        {}
        ~TelephonyStateObserver()
        {
            helper_ = nullptr;
        }
        void OnCallStateUpdated(int32_t slotId, int32_t callState, const std::u16string &phoneNumber) override;

    public:
        std::shared_ptr<TriggerHelper> helper_ = nullptr;
    };

private:
    sptr<TelephonyStateObserver> telephonyObserver0_ = nullptr;
#endif

#ifdef SUPPORT_WINDOW_MANAGER
public:
    void AttachFoldStatusListener();
    void DetachFoldStatusListener();

private:
    void UpdateGenericTriggerModel(std::shared_ptr<GenericTriggerModel> model);
    std::shared_ptr<GenericTriggerModel> ReadWhisperModel();
    void ReLoadWhisperModel(std::shared_ptr<TriggerModelData> modelData);
    void FoldStatusOperation(std::shared_ptr<TriggerModelData> modelData);
    void SetFoldStatus();
    void RegisterFoldStatusListener();
    void StartAllRecognition();
    void StopAllRecognition();
    void RestartAllRecognition();
    void OnFoldStatusChanged(FoldStatus foldStatus);
    std::string GetFoldStatusInfo();
    bool GetParameterInner(const std::string &key, std::string &value);
class FoldStatusListener : public OHOS::Rosen::DisplayManagerLite::IFoldStatusListener {
public:
    explicit FoldStatusListener(const std::shared_ptr<TriggerHelper> helper) : helper_(helper)
    {}

    ~FoldStatusListener()
    {
        helper_ = nullptr;
    }
    void OnFoldStatusChanged(FoldStatus foldStatus) override;

    public:
        std::shared_ptr<TriggerHelper> helper_ = nullptr;
};
private:
    sptr<OHOS::Rosen::DisplayManagerLite::IFoldStatusListener> foldStatusListener_;
    FoldStatus curFoldStatus_ = FoldStatus::UNKNOWN;
    bool isFoldStatusDetached_ = false;
    bool isFoldable_ = false;
    std::mutex foldStatusMutex_;
#endif

    class AudioSceneChangeCallback : public AudioManagerAudioSceneChangedCallback {
    public:
        explicit AudioSceneChangeCallback(const std::shared_ptr<TriggerHelper> helper) : helper_(helper)
        {}
        ~AudioSceneChangeCallback()
        {
            helper_ = nullptr;
        }
        void OnAudioSceneChange(const AudioScene audioScene) override;

    public:
        std::shared_ptr<TriggerHelper> helper_ = nullptr;
    };

    class AudioCapturerSourceChangeCallback : public AudioCapturerSourceCallback {
    public:
        explicit AudioCapturerSourceChangeCallback(const std::shared_ptr<TriggerHelper> helper) : helper_(helper)
        {}
        ~AudioCapturerSourceChangeCallback()
        {
            helper_ = nullptr;
        }
        void OnCapturerState(bool isActive) override;

    public:
        std::shared_ptr<TriggerHelper> helper_ = nullptr;
    };

    class AudioRendererStateChangeCallbackImpl : public AudioRendererStateChangeCallback {
    public:
        explicit AudioRendererStateChangeCallbackImpl(const std::shared_ptr<TriggerHelper> helper)
            : helper_(helper)
        {}
        ~AudioRendererStateChangeCallbackImpl()
        {
            helper_ = nullptr;
            rendererStateMap_.clear();
        }

        void OnRendererStateChange(
           const std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override;
    private:
        std::mutex mutex_;
        std::shared_ptr<TriggerHelper> helper_ = nullptr;
        std::map<int32_t, bool> rendererStateMap_;
    };

#ifdef POWER_MANAGER_ENABLE
    class HibernateCallback : public PowerMgr::SyncHibernateCallbackStub {
    public:
        explicit HibernateCallback(const std::shared_ptr<TriggerHelper> helper)
            : helper_(helper)
        {}
        ~HibernateCallback()
        {
            helper_ = nullptr;
        }

        void OnSyncHibernate() override;
        void OnSyncWakeup(bool hibernateResult = false) override;

    private:
        std::shared_ptr<TriggerHelper> helper_ = nullptr;
    };

    class SleepCallback : public PowerMgr::SyncSleepCallbackStub {
    public:
        explicit SleepCallback(const std::shared_ptr<TriggerHelper> helper)
            : helper_(helper)
        {}
        ~SleepCallback()
        {
            helper_ = nullptr;
        }

        void OnSyncSleep(bool onForceSleep);
        void OnSyncWakeup(bool onForceSleep);

    private:
        std::shared_ptr<TriggerHelper> helper_ = nullptr;
    };
#endif

private:
    std::mutex mutex_;
    std::mutex telephonyMutex_;
    std::mutex rendererMutex_;
#ifdef POWER_MANAGER_ENABLE
    std::mutex hiberateMutex_;
#endif
    std::mutex sceneMutex_;

    std::map<int32_t, std::shared_ptr<TriggerModelData>> modelDataMap_;
    std::shared_ptr<IIntellVoiceTriggerConnectorModule> module_ = nullptr;
    std::vector<TriggerConnectorModuleDesc> moduleDesc_;
    std::shared_ptr<AudioCapturerSourceChangeCallback> audioCapturerSourceChangeCallback_ = nullptr;
    std::shared_ptr<AudioRendererStateChangeCallbackImpl> audioRendererStateChangeCallback_ = nullptr;
    std::shared_ptr<AudioSceneChangeCallback> audioSceneChangeCallback_ = nullptr;
    bool callActive_ = false;
    bool systemHibernate_ = false;
    bool audioCaptureActive_ = false;
    AudioScene audioScene_ = AudioScene::AUDIO_SCENE_DEFAULT;
#ifdef SUPPORT_TELEPHONY_SERVICE
    bool isTelephonyDetached_ = false;
#endif
    bool isRendererDetached_ = false;
#ifdef POWER_MANAGER_ENABLE
    sptr<HibernateCallback> hibernateCallback_ = nullptr;
    sptr<SleepCallback> sleepCallback_ = nullptr;
    bool isHibernateDetached_ = false;
#endif
    bool isSceneDetached_ = false;
};
}  // namespace IntellVoiceTrigger
}  // namespace OHOS
#endif