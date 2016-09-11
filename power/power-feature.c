/*
 * Copyright (C) 2016 The CyanogenMod Project
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

#include <stdlib.h>
#include <hardware/power.h>

#define LOG_TAG "Kiwi/QCOM PowerHAL"
#include <utils/Log.h>

#include "power-common.h"
#include "power-feature.h"
#include "utils.h"

#define UNUSED __attribute__ ((unused))

#define TAP_TO_WAKE_NODE "/sys/touch_screen/easy_wakeup_gesture"

void set_device_specific_feature(struct power_module *module UNUSED, feature_t feature, int state)
{
    char tmp_str[NODE_MAX];
    if (feature == POWER_FEATURE_DOUBLE_TAP_TO_WAKE) {
        FILE *f;
        if ((f = fopen(TAP_TO_WAKE_NODE, "r")) != NULL) {
            if (fgets(tmp_str, sizeof(tmp_str), f) != NULL) {
                int curr_state;
                sscanf(tmp_str, "%x", &curr_state);
                ALOGD("Curr state: %d state: %d", curr_state, state);
                if (state == 0) {
                    curr_state &= ~1;
                } else {
                    curr_state |= 1;
                }
                ALOGD("Curr state: %d state: %d", curr_state, state);
                snprintf(tmp_str, NODE_MAX, "%d", curr_state);
                if (sysfs_write(TAP_TO_WAKE_NODE, tmp_str) != 0) {
                    ALOGE("Failed to write to node at %s", TAP_TO_WAKE_NODE);
                }
            } else {
                ALOGE("Failed to read from %s", TAP_TO_WAKE_NODE);
            }

            fclose(f);
        } else {
            ALOGE("Failed to open file %s", TAP_TO_WAKE_NODE);
        }
    }
}

