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

#ifdef HW_LIBC_DEBUG
#define LOG_TAG "HWAppInfo"
#include <log/log.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * References in kernel:
 * fs/proc/app_info.c
 * include/misc/app_info.h
 */

#define APP_INFO_PATH "/proc/app_info"
#define APP_INFO_VALUE_LENGTH 32

int get_app_info(char* key, char* value) {
    char buf[128] = { 0 };
    char* tok;
    FILE* f;

#ifdef HW_LIBC_DEBUG
    ALOGI("Getting App Info for %s", key);
#endif

    if(key == NULL) {
#ifdef HW_LIBC_DEBUG
        ALOGE("Key is null");
#endif
        return -1;
    }

    f = fopen(APP_INFO_PATH, "rb");
    if(f == NULL) {
#ifdef HW_LIBC_DEBUG
        ALOGE("Failed to open %s: %s", APP_INFO_PATH, strerror(errno));
#endif
        return -2;
    }

    while (!feof(f)) {
        if (fgets(buf, 128, f) != NULL &&
                strstr(buf, key) != NULL) {
            tok = strchr(buf, ':');
            if (tok != NULL)
                tok = strtok(tok, ": ");
            if (tok != NULL) {
                snprintf(value, APP_INFO_VALUE_LENGTH, "%s", tok);
                strtok(value, "\n");
            }
            break;
        }
    }

    fclose(f);

#ifdef HW_LIBC_DEBUG
    ALOGI("%s=%s", key, value);
#endif

    return 0;
}
