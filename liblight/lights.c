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


//#define LOG_NDEBUG 0
#define LOG_TAG "lights"

#include <cutils/log.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/types.h>

#include <hardware/lights.h>
#include <linux/leds-aw-leds.h>

/******************************************************************************/

static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static struct light_state_t g_attention;
static struct light_state_t g_notification;
static struct light_state_t g_battery;

const char *const AW_LED_FILE
        = "/dev/aw-leds";

const char *const LCD_FILE
        = "/sys/class/leds/lcd-backlight/brightness";

const char *const BUTTONS_FILE
        = "/sys/class/leds/button-backlight/brightness";

/**
 * device methods
 */

void init_globals(void)
{
    // init the mutex
    pthread_mutex_init(&g_lock, NULL);
}

static int
write_string(const char *path, const char *buffer)
{
    int fd;
    static int already_warned = 0;

    fd = open(path, O_RDWR);
    if (fd >= 0) {
        int bytes = strlen(buffer);
        int amt = write(fd, buffer, bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
            ALOGE("write_string failed to open %s (%s)\n", path, strerror(errno));
            already_warned = 1;
        }
        return -errno;
    }
}

static int
write_int(const char *path, int value)
{
    char buffer[20];
    sprintf(buffer, "%d\n", value);
    return write_string(path, buffer);
}

static int
rgb_to_brightness(const struct light_state_t *state)
{
    int color = state->color & 0x00ffffff;
    return ((77 * ((color >> 16) & 0xff))
            + (150 * ((color >> 8) & 0xff))
            + (29 * (color & 0xff))) >> 8;
}

static int
is_lit(const struct light_state_t *state)
{
    return (state->color & 0x00ffffff);
}

static int
aw_led_cfg(struct aw_leds_control *cfg)
{
    int fd;
    static int already_warned = 0;

    fd = open(AW_LED_FILE, O_RDWR);
    if (fd >= 0) {
        int ret = ioctl(fd, AW_LEDS_SET_LEDS, cfg);
        close(fd);
        return ret == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
            ALOGE("write_string failed to open %s (%s)\n", AW_LED_FILE, strerror(errno));
            already_warned = 1;
        }
        return -errno;
    }
}

static int
set_speaker_light_locked(struct light_device_t *dev,
        const struct light_state_t *state)
{
    struct aw_leds_control cfg;

    if (state == NULL) {
        cfg.red = cfg.blue = cfg.green = 0;
        cfg.blink = 0;
        aw_led_cfg(&cfg);
        return 0;
    }

    if (state->flashMode == LIGHT_FLASH_TIMED &&
            state->flashOnMS && state->flashOffMS) {
        cfg.blink = 1;
    } else {
        cfg.blink = 0;
    }

    //The AW LEDS driver cannot currently set RGB values < 255
    //while blinking.  Set to a reasonable color.
    cfg.blue = ((state->color) & 0xff) >= 128 ? 255 : 0;
    cfg.green = ((state->color >> 8) & 0xff) >= 128 ? 255 : 0;
    cfg.red = ((state->color >> 16) & 0xff) >= 128 ? 255 : 0;
    aw_led_cfg(&cfg);
    return 0;
}

static void
handle_speaker_light_locked(struct light_device_t *dev)
{
    set_speaker_light_locked(dev, NULL);
    if (is_lit(&g_attention)) {
        set_speaker_light_locked(dev, &g_attention);
    } else if (is_lit(&g_notification)) {
        set_speaker_light_locked(dev, &g_notification);
    } else {
        set_speaker_light_locked(dev, &g_battery);
    }
}

static int
set_light_backlight(struct light_device_t *dev,
        const struct light_state_t *state)
{
    int err = 0;
    int brightness = rgb_to_brightness(state);

    pthread_mutex_lock(&g_lock);

    err = write_int(LCD_FILE, brightness);

    pthread_mutex_unlock(&g_lock);

    return err;
}

static int
set_light_buttons(struct light_device_t *dev,
        const struct light_state_t *state)
{
    int err = 0;
    int brightness = rgb_to_brightness(state);

    pthread_mutex_lock(&g_lock);

    err = write_int(BUTTONS_FILE, brightness);

    pthread_mutex_unlock(&g_lock);

    return err;
}

static int
set_light_attention(struct light_device_t *dev,
        const struct light_state_t *state)
{
    pthread_mutex_lock(&g_lock);

    g_attention = *state;
    handle_speaker_light_locked(dev);

    pthread_mutex_unlock(&g_lock);

    return 0;
}

static int
set_light_notifications(struct light_device_t *dev,
        const struct light_state_t *state)
{
    pthread_mutex_lock(&g_lock);

    g_notification = *state;
    handle_speaker_light_locked(dev);

    pthread_mutex_unlock(&g_lock);

    return 0;
}

static int
set_light_battery(struct light_device_t *dev,
        const struct light_state_t *state)
{
    pthread_mutex_lock(&g_lock);

    g_battery = *state;
    handle_speaker_light_locked(dev);

    pthread_mutex_unlock(&g_lock);

    return 0;
}

/** Close the lights device */
static int
close_lights(struct light_device_t *dev)
{
    if (dev) {
        free(dev);
    }
    return 0;
}

/******************************************************************************/

/**
 * module methods
 */

/** Open a new instance of a lights device using name */
static int open_lights(const struct hw_module_t *module, const char *name,
        struct hw_device_t **device)
{
    int (*set_light)(struct light_device_t *dev,
            const struct light_state_t *state);

    if (0 == strcmp(LIGHT_ID_BACKLIGHT, name))
        set_light = set_light_backlight;
    else if (0 == strcmp(LIGHT_ID_BUTTONS, name))
        set_light = set_light_buttons;
    else if (0 == strcmp(LIGHT_ID_NOTIFICATIONS, name))
        set_light = set_light_notifications;
    else if (0 == strcmp(LIGHT_ID_ATTENTION, name))
        set_light = set_light_attention;
    else if (0 == strcmp(LIGHT_ID_BATTERY, name))
        set_light = set_light_battery;
    else
        return -EINVAL;

    pthread_once(&g_init, init_globals);

    struct light_device_t *dev = malloc(sizeof(struct light_device_t));
    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))close_lights;
    dev->set_light = set_light;

    *device = (struct hw_device_t*)dev;
    return 0;
}

static struct hw_module_methods_t lights_module_methods = {
    .open =  open_lights,
};

/*
 * The lights Module
 */
struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "YU Lights Module",
    .author = "The CyanogenMod Project",
    .methods = &lights_module_methods,
};
