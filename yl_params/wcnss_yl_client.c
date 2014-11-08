/*
 * Copyright (C) 2014, The CyanogenMod Project
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

#define LOG_TAG "wcnss_yl"

#define SUCCESS 0
#define FAILED -1

#include <cutils/log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "yl_params.h"

int wcnss_init_qmi(void)
{
    /* empty */
    return SUCCESS;
}

int wcnss_qmi_get_wlan_address(unsigned char *pBdAddr)
{
    int rc;
    char buf[6];

    rc = yl_params_init();
    if (rc) {
        ALOGE("%s: Failed to initialize yl_params: %d\n", __func__, rc);
        return FAILED;
    }

    rc = yl_get_param(YL_PARAM_WLAN_MAC, buf, 6);
    if (rc) {
        ALOGE("%s: Failed to retrieve MAC from yl_params: %d\n", __func__, rc);
        return FAILED;
    }

    memcpy(pBdAddr, buf, 6);

    ALOGI("%s: Got addr: %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
            __func__,
            pBdAddr[0], pBdAddr[1], pBdAddr[2],
            pBdAddr[3], pBdAddr[4], pBdAddr[5]);

    return SUCCESS;
}

void wcnss_qmi_deinit(void)
{
    /* empty */
}
