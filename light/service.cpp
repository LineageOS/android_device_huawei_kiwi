/*
 * Copyright 2017 The LineageOS Project
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

#define LOG_TAG "android.hardware.light@2.0-service.kiwi"

#include <dlfcn.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/Errors.h>

#include "Light.h"

#define QMI_OEM_API_LIB_NAME "libqmi_oem_api.so"

// libhwbinder:
using android::OK;
using android::status_t;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

// Generated HIDL files
using android::hardware::light::V2_0::ILight;
using android::hardware::light::V2_0::implementation::Light;

const static std::string kBacklightPath = "/sys/class/leds/lcd-backlight/brightness";

int main() {
    android::sp<ILight> service;
    status_t status = OK;
    void* qmiApi;

    std::ofstream backlight(kBacklightPath);
    if (!backlight) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kBacklightPath.c_str(), error, strerror(error));
        return -error;
    }

    /*
     * Open the qmi api.
     */
    qmiApi = dlopen(QMI_OEM_API_LIB_NAME, RTLD_NOW);
    if (!qmiApi) {
        ALOGE("%s: failed to load %s: %s\n", __func__, QMI_OEM_API_LIB_NAME, dlerror());
        goto shutdown;
    }
    
    service = new Light(std::move(backlight), qmiApi);

    configureRpcThreadpool(1, true);

    status = service->registerAsService();
    if (status != android::OK) {
        ALOGE("Cannot register Light HAL service");
        return 1;
    }

    ALOGI("Light HAL Ready.");
    joinRpcThreadpool();

shutdown:
    if (qmiApi != nullptr) {
        dlclose(qmiApi);
    }

    // Under normal cases, execution will not reach this line.
    ALOGE("Light HAL failed to join thread pool.");

    return 1;
}
