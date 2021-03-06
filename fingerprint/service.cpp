/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "android.hardware.biometrics.fingerprint@1.0-service.kiwi"

#include <android/log.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <android/hardware/biometrics/fingerprint/2.1/types.h>
#include <cstring>
#include "BiometricsFingerprint.h"

using android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
using android::hardware::biometrics::fingerprint::V2_1::implementation::BiometricsFingerprint;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::sp;

static void setProcessTitle(char* currentTitle, const char* newTitle) {
    auto currentLength = std::strlen(currentTitle);
    auto newLength = std::strlen(newTitle);

    if (currentLength < newLength) {
        ALOGE("Cannot set new process title");
        return;
    }

    std::memset(currentTitle, 0, currentLength);
    std::memcpy(currentTitle, newTitle, newLength);
}

int main(int, char** argv) {
    setProcessTitle(argv[0], "/system/bin/fingerprintd");
    android::sp<IBiometricsFingerprint> bio = BiometricsFingerprint::getInstance();

    configureRpcThreadpool(1, true /*callerWillJoin*/);

    if (bio == nullptr) {
        ALOGE("Can't create instance of BiometricsFingerprint, nullptr");
    } else if (bio->registerAsService() != android::OK) {
        ALOGE("Cannot register kiwi fingerprint service");
    }

    joinRpcThreadpool();

    return 0; // should never get here
}
