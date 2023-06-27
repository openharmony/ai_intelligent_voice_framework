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

import { AsyncCallback, Callback } from './@ohos.base';

/**
 * @namespace intelligentVoice
 * @since 10
 */
declare namespace intelligentVoice {
  /**
   * Obtains an {@link IntelligentVoiceManager} instance.
   * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
   * @returns { IntelligentVoiceManager } this {@link IntelligentVoiceManager} object.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 22700101 - No memory.
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  function getIntelligentVoiceManager(): IntelligentVoiceManager;

  /**
   * Implements intelligent voice management.
   * @typedef IntelligentVoiceManager
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  interface IntelligentVoiceManager {
    /**
     * Obtains capability information.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @returns { Array<IntelligentVoiceEngineType> } array of supported IntelligentVoiceEngineType.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    getCapabilityInfo(): Array<IntelligentVoiceEngineType>;
    /**
     * Subscribes service change events. When the state of intelligent voice service changes,
     * the callback is invoked.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { string } type - Type of the event to listen for. Only the serviceChange event is supported.
     * @param { Callback<ServiceChangeType>Callback<ServiceChangeType> } callback - Callback is invoked when the event is triggered.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    on(type: 'serviceChange', callback: Callback<ServiceChangeType>): void;
    /**
     * Unsubscribes service change events.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { string } type - Type of the event to listen for. Only the serviceChange event is supported.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    off(type: 'serviceChange'): void;
  }

  /**
   * Enumerates service change type.
   * @enum {number}
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  enum ServiceChangeType {
    /**
     * Service unavailable.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    SERVICE_UNAVAILABLE = 0,
  }

  /**
   * Enumerates intelligent voice engine type.
   * @enum {number}
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  enum IntelligentVoiceEngineType {
    /**
     * Enroll engine.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    ENROLL_ENGINE_TYPE = 0,
    /**
     * Wakeup engine.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    WAKEUP_ENGINE_TYPE = 1,
    /**
     * Update engine.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    UPDATE_ENGINE_TYPE = 2,
  }

  /**
   * Describes enroll intelligent voice engine.
   * @typedef EnrollIntelligentVoiceEngineDescriptor
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  interface EnrollIntelligentVoiceEngineDescriptor {
    /**
     * Wakeup phrase.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    wakeupPhrase: string;
  }

  /**
   * Describes wakeup intelligent voice engine.
   * @typedef WakeupIntelligentVoiceEngineDescriptor
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  interface WakeupIntelligentVoiceEngineDescriptor {
    /**
     * Need ap algorithm engine.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    needApAlgEngine: boolean;
    /**
     * Wakeup phrase.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    wakeupPhrase: string;
  }

  /**
   * Obtains an {@link EnrollIntelligentVoiceEngine} instance. This method uses an asynchronous callback to return the EnrollIntelligentVoiceEngine instance.
   * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
   * @param { EnrollIntelligentVoiceEngineDescriptor } descriptor - descriptor indicates enroll intelligent voice engine descriptor.
   * @param { AsyncCallback<EnrollIntelligentVoiceEngine> } callback - the callback used to return the EnrollIntelligentVoiceEngine instance.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - if input parameter type or number mismatch.
   * @throws { BusinessError } 22700101 - No memory.
   * @throws { BusinessError } 22700102 - if input parameter value error.
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  function createEnrollIntelligentVoiceEngine(descriptor: EnrollIntelligentVoiceEngineDescriptor, callback: AsyncCallback<EnrollIntelligentVoiceEngine>): void;

  /**
   * Obtains an {@link EnrollIntelligentVoiceEngine} instance. This method uses a promise to return the EnrollIntelligentVoiceEngine instance.
   * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
   * @param { EnrollIntelligentVoiceEngineDescriptor } descriptor - descriptor indicates enroll intelligent voice engine descriptor.
   * @returns { Promise<EnrollIntelligentVoiceEngine>; } the promise used to return the EnrollIntelligentVoiceEngine instance.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - if input parameter type or number mismatch.
   * @throws { BusinessError } 22700101 - No memory.
   * @throws { BusinessError } 22700102 - if input parameter value error.
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  function createEnrollIntelligentVoiceEngine(descriptor: EnrollIntelligentVoiceEngineDescriptor): Promise<EnrollIntelligentVoiceEngine>;

  /**
   * Obtains an {@link WakeupIntelligentVoiceEngine} instance. This method uses an asynchronous callback to return the WakeupIntelligentVoiceEngine instance.
   * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
   * @param { WakeupIntelligentVoiceEngineDescriptor } descriptor - descriptor indicates wakeup intelligent voice engine descriptor.
   * @param { AsyncCallback<WakeupIntelligentVoiceEngine> } callback - the callback used to return the WakeupIntelligentVoiceEngine instance.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - if input parameter type or number mismatch.
   * @throws { BusinessError } 22700101 - No memory.
   * @throws { BusinessError } 22700102 - if input parameter value error.
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  function createWakeupIntelligentVoiceEngine(descriptor: WakeupIntelligentVoiceEngineDescriptor, callback: AsyncCallback<WakeupIntelligentVoiceEngine>): void;

  /**
   * Obtains an {@link WakeupIntelligentVoiceEngine} instance. This method uses a promise to return the WakeupIntelligentVoiceEngine instance.
   * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
   * @param { WakeupIntelligentVoiceEngineDescriptor } descriptor - descriptor indicates wakeup intelligent voice engine descriptor.
   * @returns { Promise<WakeupIntelligentVoiceEngine> } the promise used to return the WakeupIntelligentVoiceEngine instance.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - if input parameter type or number mismatch.
   * @throws { BusinessError } 22700101 - No memory.
   * @throws { BusinessError } 22700102 - if input parameter value error.
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  function createWakeupIntelligentVoiceEngine(descriptor: WakeupIntelligentVoiceEngineDescriptor): Promise<WakeupIntelligentVoiceEngine>;

  /**
   * Describes enroll engine config.
   * @typedef EnrollEngineConfig
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  interface EnrollEngineConfig {
    /**
     * Language that enroll engine supports.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    language: string;
    /**
     * Region that enroll engine supports.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    region: string;
  }

  /**
   * Enumerates sensibility type.
   * @enum {number}
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  enum SensibilityType {
    /**
     * Low sensibility.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    LOW_SENSIBILITY = 1,
    /**
     * Middle sensibility.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    MIDDLE_SENSIBILITY = 2,
    /**
     * High sensibility.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    HIGH_SENSIBILITY = 3,
  }

  /**
   * Describes wakeup hap information.
   * @typedef WakeupHapInfo
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  interface WakeupHapInfo {
    /**
     * Bundle name.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    bundleName: string;
    /**
     * Ability name.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    abilityName: string;
  }

  /**
   * Enumerates enroll intelligent voice event type.
   * @enum {number}
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  enum EnrollIntelligentVoiceEventType {
    /**
     * Enroll None.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    INTELLIGENT_VOICE_EVENT_ENROLL_NONE = 0,
    /**
     * Init done.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    INTELLIGENT_VOICE_EVENT_ENROLL_INIT_DONE = 1,
    /**
     * Enroll complete.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    INTELLIGENT_VOICE_EVENT_ENROLL_COMPLETE = 2,
    /**
     * Commit enroll complete.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    INTELLIGENT_VOICE_EVENT_COMMIT_ENROLL_COMPLETE = 3,
  }

  /**
   * Enumerates wakeup intelligent voice event type.
   * @enum {number}
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  enum WakeupIntelligentVoiceEventType {
    /**
     * Wakeup None.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    INTELLIGENT_VOICE_EVENT_WAKEUP_NONE = 0,
    /**
     * Recognize complete.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    INTELLIGENT_VOICE_EVENT_RECOGNIZE_COMPLETE = 1,
  }

  /**
   * Enumerates intelligent voice error code.
   * @enum {number}
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  enum IntelligentVoiceErrorCode {
    /**
     * Success.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    INTELLIGENT_VOICE_SUCCESS = 0,
    /**
     * Memory is insufficient.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    INTELLIGENT_VOICE_NO_MEMORY = 22700101,
    /**
     * Invalid parameter.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    INTELLIGENT_VOICE_INVALID_PARAM = 22700102,
    /**
     * Init failed.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    INTELLIGENT_VOICE_INIT_FAILED = 22700103,
    /**
     * Enroll failed.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    INTELLIGENT_VOICE_ENROLL_FAILED = 22700104,
    /**
     * Commit enroll failed.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    INTELLIGENT_VOICE_COMMIT_ENROLL_FAILED = 22700105,
    /**
     * Recognize failed.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    INTELLIGENT_VOICE_RECOGNIZE_FAILED = 22700106,
  }

  /**
   * Describes enrol intelligent voice engine callback information.
   * @typedef EnrollIntelligentVoiceEngineCallbackInfo
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  interface EnrollIntelligentVoiceEngineCallbackInfo {
    /**
     * Enroll event id.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    eventId: EnrollIntelligentVoiceEventType;
    /**
     * Error code.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    errCode: IntelligentVoiceErrorCode;
    /**
     * Describes enroll event context.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    context: string;
  }

  /**
   * Describes wakeup intelligent voice engine callback information.
   * @typedef WakeupIntelligentVoiceEngineCallbackInfo
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  interface WakeupIntelligentVoiceEngineCallbackInfo {
    /**
     * Wakeup event id.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    eventId: WakeupIntelligentVoiceEventType;
    /**
     * Error code.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    errCode: IntelligentVoiceErrorCode;
    /**
     * Describes wakeup event context.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    context: string;
  }

  /**
   * Implements enroll intelligent voice engine.
   * @typedef EnrollIntelligentVoiceEngine
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  interface EnrollIntelligentVoiceEngine {
    /**
     * Obtains the supported regions, This method uses an asynchronous callback to return the query result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { AsyncCallback<Array<string>> } callback - the callback used to return the supported regions.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    getSupportedRegions(callback: AsyncCallback<Array<string>>): void;
    /**
     * Obtains the supported regions, This method uses a promise to return the query result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @returns { Promise<Array<string>> } the promise used to return the supported regions.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    getSupportedRegions(): Promise<Array<string>>;
    /**
     * Initials the engine, This method uses an asynchronous callback to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { EnrollEngineConfig } config - config indicates enroll engine configuration.
     * @param { AsyncCallback<EnrollIntelligentVoiceEngineCallbackInfo> } callback - the callback used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    init(config: EnrollEngineConfig, callback: AsyncCallback<EnrollIntelligentVoiceEngineCallbackInfo>): void;
    /**
     * Initials the engine, This method uses a promise to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { EnrollEngineConfig } config - config indicates enroll engine configuration.
     * @returns { Promise<EnrollIntelligentVoiceEngineCallbackInfo> } the promise used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    init(config: EnrollEngineConfig): Promise<EnrollIntelligentVoiceEngineCallbackInfo>;
    /**
     * Starts the engine, This method uses an asynchronous callback to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { boolean } isLast - isLast indicates if it is the last time to start.
     * @param { AsyncCallback<EnrollIntelligentVoiceEngineCallbackInfo> } callback - the callback used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    start(isLast: boolean, callback: AsyncCallback<EnrollIntelligentVoiceEngineCallbackInfo>): void;
    /**
     * Starts the engine, This method uses a promise to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { boolean } isLast - isLast indicates if it is the last time to start.
     * @returns { Promise<EnrollIntelligentVoiceEngineCallbackInfo> } the promise used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    start(isLast: boolean): Promise<EnrollIntelligentVoiceEngineCallbackInfo>;
    /**
     * Stops the engine, This method uses an asynchronous callback to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { AsyncCallback<void> } callback  - the callback used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    stop(callback: AsyncCallback<void>): void;
    /**
     * Stops the engine, This method uses a promise to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @returns { Promise<void> } the promise used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    stop(): Promise<void>;
    /**
     * commit enroll, This method uses an asynchronous callback to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { AsyncCallback<EnrollIntelligentVoiceEngineCallbackInfo> } callback - the callback used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    commit(callback: AsyncCallback<EnrollIntelligentVoiceEngineCallbackInfo>): void;
    /**
     * commit enroll, This method uses a promise to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @returns { Promise<EnrollIntelligentVoiceEngineCallbackInfo> } the promise used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    commit(): Promise<EnrollIntelligentVoiceEngineCallbackInfo>;
    /**
     * Sets wakeup hap information, This method uses an asynchronous callback to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { WakeupHapInfo } info - info indicates wakeup hap information.
     * @param { AsyncCallback<void> } callback - the callback used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    setWakeupHapInfo(info: WakeupHapInfo, callback: AsyncCallback<void>): void;
    /**
     * Sets wakeup hap information, This method uses a promise to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { WakeupHapInfo } info - info indicates wakeup hap information.
     * @returns { Promise<void> } the promise used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    setWakeupHapInfo(info: WakeupHapInfo): Promise<void>;
    /**
     * Sets sensibility, This method uses an asynchronous callback to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { SensibilityType } sensibility - sensibility to set.
     * @param { AsyncCallback<void> } callback - the callback used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    setSensibility(sensibility: SensibilityType, callback: AsyncCallback<void>): void;
    /**
     * Sets sensibility, This method uses a promise to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { SensibilityType } sensibility - sensibility to set.
     * @returns { Promise<void> } the promise used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    setSensibility(sensibility: SensibilityType): Promise<void>;
    /**
     * Sets an intelligent voice parameter. This method uses an asynchronous callback to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { string } key - the key of the intelligent voice parameter to set.
     * @param { string } value - the value of the intelligent voice parameter to set.
     * @param { AsyncCallback<void> } callback - the callback used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    setParameter(key: string, value: string, callback: AsyncCallback<void>): void;
    /**
     * Sets an intelligent voice parameter. This method uses a promise to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { string } key - the key of the intelligent voice parameter to set.
     * @param { string } value - the value of the intelligent voice parameter to set.
     * @returns { Promise<void> } the promise used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    setParameter(key: string, value: string): Promise<void>;
    /**
     * Obtains the value of an intelligent voice parameter. This method uses an asynchronous callback to return the query result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { string } key - the key of the intelligent voice parameter whose value is to be obtained.
     * @param { AsyncCallback<string> } callback - the callback used to return the value of the intelligent voice parameter.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    getParameter(key: string, callback: AsyncCallback<string>): void;
    /**
     * Obtains the value of an intelligent voice parameter. This method uses an asynchronous callback to return the query result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { string } key - the key of the intelligent voice parameter whose value is to be obtained.
     * @returns { Promise<string> } the promise used to return the value of the intelligent voice parameter.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    getParameter(key: string): Promise<string>;
    /**
     * Releases the engine, This method uses an asynchronous callback to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { AsyncCallback<void> } callback - the callback used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    release(callback: AsyncCallback<void>): void;
    /**
     * Releases the engine, This method uses a promise to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @returns { Promise<void> } the promise used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    release(): Promise<void>;
  }

  /**
   * Implements wakeup intelligent voice engine.
   * @typedef WakeupIntelligentVoiceEngine
   * @syscap SystemCapability.AI.IntelligentVoice.Core
   * @since 10
   */
  interface WakeupIntelligentVoiceEngine {
    /**
     * Obtains the supported regions, This method uses an asynchronous callback to return the query result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { AsyncCallback<Array<string>> } callback - the callback used to return the supported regions.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    getSupportedRegions(callback: AsyncCallback<Array<string>>): void;
    /**
     * Obtains the supported regions, This method uses a promise to return the query result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @returns { Promise<Array<string>> } the promise used to return the supported regions.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    getSupportedRegions(): Promise<Array<string>>;
    /**
     * Sets wakeup hap information, This method uses an asynchronous callback to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { WakeupHapInfo } info - info indicates wakeup hap information.
     * @param { AsyncCallback<void> } callback - the callback used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    setWakeupHapInfo(info: WakeupHapInfo, callback: AsyncCallback<void>): void;
    /**
     * Sets wakeup hap information, This method uses a promise to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { WakeupHapInfo } info - info indicates wakeup hap information.
     * @returns { Promise<void> } the promise used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    setWakeupHapInfo(info: WakeupHapInfo): Promise<void>;
    /**
     * Sets sensibility, This method uses an asynchronous callback to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { SensibilityType } sensibility - sensibility to set.
     * @param { AsyncCallback<void> } callback - the callback used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    setSensibility(sensibility: SensibilityType, callback: AsyncCallback<void>): void;
    /**
     * Sets sensibility, This method uses a promise to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { SensibilityType } sensibility - sensibility to set.
     * @returns { Promise<void> } the promise used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    setSensibility(sensibility: SensibilityType): Promise<void>;
    /**
     * Sets an intelligent voice parameter. This method uses an asynchronous callback to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { string } key - the key of the intelligent voice parameter to set.
     * @param { string } value - the value of the intelligent voice parameter to set.
     * @param { AsyncCallback<void> } callback - the callback used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    setParameter(key: string, value: string, callback: AsyncCallback<void>): void;
    /**
     * Sets an intelligent voice parameter. This method uses a promise to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { string } key - the key of the intelligent voice parameter to set.
     * @param { string } value - the value of the intelligent voice parameter to set.
     * @returns { Promise<void> } the promise used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    setParameter(key: string, value: string): Promise<void>;
    /**
     * Obtains the value of an intelligent voice parameter. This method uses an asynchronous callback to return the query result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { string } key - the key of the intelligent voice parameter whose value is to be obtained.
     * @param { AsyncCallback<string> } callback - the callback used to return the value of the intelligent voice parameter.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    getParameter(key: string, callback: AsyncCallback<string>): void;
    /**
     * Obtains the value of an intelligent voice parameter. This method uses an asynchronous callback to return the query result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { string } key - the key of the intelligent voice parameter whose value is to be obtained.
     * @returns { Promise<string> } the promise used to return the value of the intelligent voice parameter.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - if input parameter type or number mismatch.
     * @throws { BusinessError } 22700102 - if input parameter value error.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    getParameter(key: string): Promise<string>;
    /**
     * Releases the engine, This method uses an asynchronous callback to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { AsyncCallback<void> } callback - the callback used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    release(callback: AsyncCallback<void>): void;
    /**
     * Releases the engine, This method uses a promise to return the result.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @returns { Promise<void> } the promise used to return the result.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    release(): Promise<void>;
    /**
     * Subscribes wakeup intelligent voice events. When wakeup intelligent voice events reach,
     * the callback is invoked.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { string } type - Type of the event to listen for. Only the wakeupIntelligentVoice event is supported.
     * @param { Callback<WakeupIntelligentVoiceEngineCallbackInfo> } callback - the callback invoked when the event is triggered.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    on(type: 'wakeupIntelligentVoiceEvent', callback: Callback<WakeupIntelligentVoiceEngineCallbackInfo>): void;
    /**
     * Unsubscribes wakeup intelligent voice events.
     * @permission ohos.permission.MANAGER_INTELLIGENT_VOICE
     * @param { string } type - Type of the event to listen for. Only the wakeupIntelligentVoice event is supported.
     * @throws { BusinessError } 201 - Permission denied.
     * @syscap SystemCapability.AI.IntelligentVoice.Core
     * @since 10
     */
    off(type: 'wakeupIntelligentVoiceEvent');
  }
}

export default intelligentVoice;