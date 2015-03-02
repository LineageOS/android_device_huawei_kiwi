/*
 * Copyright (C) 2015 The CyanogenMod Project
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

#define LOG_TAG "jni_proximityCalibrate"

#include <fcntl.h>
#include <jni.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cutils/log.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define UNUSED __attribute__((unused))

#define ALSPROX_DEVICE_NAME          "/dev/yl_alsprox_sensor"

#define ALSPROX_IOCTL_MAGIC          (0xCF)
#define ALSPROX_IOCTL_ALS_ON         _IOW(ALSPROX_IOCTL_MAGIC, 1, unsigned long)
#define ALSPROX_IOCTL_ALS_OFF        _IOW(ALSPROX_IOCTL_MAGIC, 2, unsigned long)
#define ALSPROX_IOCTL_PROX_ON        _IOW(ALSPROX_IOCTL_MAGIC, 3, unsigned long)
#define ALSPROX_IOCTL_PROX_OFF       _IOW(ALSPROX_IOCTL_MAGIC, 4, unsigned long)
#define ALSPROX_IOCTL_PROX_OFFSET    _IOW(ALSPROX_IOCTL_MAGIC, 5, unsigned long)
#define ALSPROX_IOCTL_PROX_CALIBRATE _IOW(ALSPROX_IOCTL_MAGIC, 6, unsigned long)
#define ALSPROX_IOCTL_PHONE_STATE    _IOW(ALSPROX_IOCTL_MAGIC, 7, unsigned long)

struct prox_offset_t {
    uint16_t result;
    uint16_t hi;
    uint16_t lo;
    uint16_t xtalk;
    uint16_t offset;
};

JNIEXPORT jboolean JNICALL Java_org_cyanogenmod_sensors_ProximityCalibrateActivity_native_1calibrateProximity(
        JNIEnv *env, UNUSED jclass clazz, jintArray intArray)
{
    jboolean result = false;
    int32_t *array = (*env)->GetIntArrayElements(env, intArray, NULL);
    int len = (int) (*env)->GetArrayLength(env, intArray);
    struct prox_offset_t offset;
    int fd, ret;

    if (len != 5) {
        ALOGE("%s:%d: insufficient array space\n", __func__, __LINE__);
        result = false;
        goto err_release;
    }

    fd = open(ALSPROX_DEVICE_NAME, O_RDWR);
    if (fd < 0) {
        ret = -errno;
        ALOGE("%s:%d: failed to open %s: %d\n",
                __func__, __LINE__, ALSPROX_DEVICE_NAME, ret);
        result = false;
        goto err_release;
    }

    memset(&offset, 0, sizeof(struct prox_offset_t));
    ret = ioctl(fd, ALSPROX_IOCTL_PROX_CALIBRATE, &offset);
    if (ret < 0) {
        ret = -errno;
        ALOGE("%s:%d: ALSPROX_IOCTL_PROX_CALIBRATE failed: %d\n",
                __func__, __LINE__, ret);
        result = false;
        goto err_close;
    }

    array[0] = offset.hi;
    array[1] = offset.lo;
    array[2] = offset.xtalk;
    array[3] = offset.result;
    array[4] = offset.offset;

    result = (offset.result == 1) ? true : false;

err_close:
    close(fd);
err_release:
    (*env)->ReleaseIntArrayElements(env, intArray, array, 0);

    return result;
}
