/*
 * Copyright (C) 2014 The CyanogenMod Project
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

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <linux/color_enhancement.h>

#define LOG_TAG "CMHW"
#include <cutils/log.h>

#define CE_DEVICE "/dev/color_enhancement"

static int sendCommand(int num, void* param)
{
    int fd = -1;

    fd = open(CE_DEVICE, O_RDWR);
    if (fd < 1) {
        ALOGE("Unable to open CE device %d", errno);
        return -1;
    }

    if (ioctl(fd, num, param) < 0) {
        ALOGE("CE IOCTL failed %d", errno);
    }

    close(fd);
    return 0;
}

static int getMode()
{
    int fd = -1;
    int mode = -1;

    fd = open(CE_DEVICE, O_RDWR);
    if (fd < 1) {
        ALOGE("Unable to open CE device %d", errno);
        return -1;
    }

    if (ioctl(fd, CE_GET_MODE, &mode) < 0) {
        ALOGE("CE IOCTL failed %d", errno);
    }

    ALOGI("Current mode: %x", mode);

    close(fd);

    return mode;
}

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL Java_org_cyanogenmod_hardware_ColorEnhancement_native_1getCELevel(
        JNIEnv* env, jclass thiz)
{
    int level = 0;
    int mode = getMode();

    if (mode & LEVEL1)
        level = 1;
    else if (mode & LEVEL2)
        level = 2;
    else if (mode & DEFAULT)
        level = 3;

    return (jint)level;
}

JNIEXPORT jint JNICALL Java_org_cyanogenmod_hardware_ColorEnhancement_native_1setCELevel(
        JNIEnv* env, jclass thiz, jint value)
{
    int level = int(value);

    ALOGI("Set color enhancement = %d", level);

    if (level >=0 && level <= 3)
        return (jint)sendCommand(CE_SET_LEVEL, &level);
    
    ALOGE("Invalid CE level %d", level);
    return -1;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    return JNI_VERSION_1_4;
}

#ifdef __cplusplus
}
#endif
