/*
 * Copyright (C) 2015, The CyanogenMod Project
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

#define LOG_TAG "audio_amplifier"
//#define LOG_NDEBUG 0

#include <stdio.h>
#include <cutils/log.h>
#include <cutils/str_parms.h>

#include <hardware/audio_amplifier.h>
#include <system/audio.h>

#include <tinyalsa/asoundlib.h>

typedef struct amp_device {
    amplifier_device_t amp_dev;
    audio_mode_t mode;
    int active;
} amp_device_t;

static amp_device_t *amp_dev = NULL;

extern int exTfa98xx_calibration(void);
extern int exTfa98xx_speakeron(uint32_t);
extern int exTfa98xx_speakeroff();
extern int exTfa98xx_set_volume(float, float);
extern int exTfa98xx_setvolumestep(uint32_t, uint32_t);

#define AMP_MIXER_CTL "Initial external PA"

static int set_clocks_enabled(bool enable)
{
    enum mixer_ctl_type type;
    struct mixer_ctl *ctl;
    struct mixer *mixer = mixer_open(0);

    if (mixer == NULL) {
        ALOGE("Error opening mixer 0");
        return -1;
    }

    ctl = mixer_get_ctl_by_name(mixer, AMP_MIXER_CTL);
    if (ctl == NULL) {
        mixer_close(mixer);
        ALOGE("%s: Could not find %s\n", __func__, AMP_MIXER_CTL);
        return -ENODEV;
    }

    type = mixer_ctl_get_type(ctl);
    if (type != MIXER_CTL_TYPE_ENUM) {
        ALOGE("%s: %s is not supported\n", __func__, AMP_MIXER_CTL);
        mixer_close(mixer);
        return -ENOTTY;
    }

    mixer_ctl_set_value(ctl, 0, enable);
    mixer_close(mixer);
    return 0;
}

static int amp_set_mode(amp_device_t *device, audio_mode_t mode)
{
    int ret = 0;
    amp_device_t *dev = (amp_device_t *) device;

    dev->mode = mode;
    return ret;
}

static int amp_set_output_devices(amp_device_t *device, uint32_t devices)
{
    amp_device_t *dev = (amp_device_t *) device;

    return 0;
}

static int amp_enable_output_devices(amp_device_t *device,
        uint32_t devices, bool enable)
{
    amp_device_t *dev = (amp_device_t *) device;
    /* TODO: 0, 1, 2 all look like they are use cases, map to something reasonable */
    int new_active = enable ? 2 : -1;

    if (new_active == dev->active) {
        ALOGI("%s: active mode unchanged from %d\n", dev->active);
    } else if (new_active >= 0) {
        set_clocks_enabled(true);
        exTfa98xx_setvolumestep(1, 1);
        exTfa98xx_speakeron(new_active);
    } else {
        exTfa98xx_speakeroff();
        set_clocks_enabled(false);
    }
    dev->active = new_active;

    return 0;
}

static int amp_dev_close(hw_device_t *device)
{
    amp_device_t *dev = (amp_device_t *) device;

    free(dev);

    return 0;
}

static int amp_module_open(const hw_module_t *module, const char *name,
        hw_device_t **device)
{
    if (amp_dev) {
        ALOGE("%s:%d: Unable to open second instance of the amplifier\n",
                __func__, __LINE__);
        return -EBUSY;
    }

    amp_dev = calloc(1, sizeof(amp_device_t));
    if (!amp_dev) {
        ALOGE("%s:%d: Unable to allocate memory for amplifier device\n",
                __func__, __LINE__);
        return -ENOMEM;
    }

    amp_dev->amp_dev.common.tag = HARDWARE_DEVICE_TAG;
    amp_dev->amp_dev.common.module = (hw_module_t *) module;
    amp_dev->amp_dev.common.version = HARDWARE_DEVICE_API_VERSION(1, 0);
    amp_dev->amp_dev.common.close = amp_dev_close;

    amp_dev->amp_dev.set_input_devices = NULL;
    amp_dev->amp_dev.set_output_devices = amp_set_output_devices;
    amp_dev->amp_dev.enable_input_devices = NULL;
    amp_dev->amp_dev.enable_output_devices = amp_enable_output_devices;
    amp_dev->amp_dev.set_mode = amp_set_mode;
    amp_dev->amp_dev.output_stream_start = NULL;
    amp_dev->amp_dev.input_stream_start = NULL;
    amp_dev->amp_dev.output_stream_standby = NULL;
    amp_dev->amp_dev.input_stream_standby = NULL;

    *device = (hw_device_t *) amp_dev;

    amp_dev->active = -1;

    set_clocks_enabled(true);
    exTfa98xx_calibration();
    set_clocks_enabled(false);

    return 0;
}

static struct hw_module_methods_t hal_module_methods = {
    .open = amp_module_open,
};

amplifier_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = AMPLIFIER_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = AMPLIFIER_HARDWARE_MODULE_ID,
        .name = "Kiwi audio amplifier HAL",
        .author = "The CyanogenMod Open Source Project",
        .methods = &hal_module_methods,
    },
};
