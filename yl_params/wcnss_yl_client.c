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

#ifndef WCNSS_GENMAC_FILE
#define WCNSS_GENMAC_FILE "/persist/.genmac"
#endif

#ifndef WCNSS_INVALID_MAC_PREFIX
#define WCNSS_INVALID_MAC_PREFIX "3c9157"
#endif

static int yl_get_wlan_address(unsigned char *buf, size_t len)
{
    int rc;

    memset(buf, 0, len);

    if (len < 6) {
        return FAILED;
    }

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

    return SUCCESS;
}

static int read_mac(unsigned char *buf, size_t len)
{
    struct stat statbuf;
    FILE *genmac;

    if (!stat(WCNSS_GENMAC_FILE, &statbuf)) {
        genmac = fopen(WCNSS_GENMAC_FILE,"r");
        if (fscanf(genmac, "%c%c%c%c%c%c", &buf[0],
                    &buf[1], &buf[2], &buf[3],
                    &buf[4], &buf[5]) == 6) {
            fclose(genmac);
            ALOGI("%s: Succesfully read local WLAN MAC addr", __func__);
            return SUCCESS;
        }
        fclose(genmac);
    }

    return FAILED;
}

static int write_mac(unsigned char *buf, size_t len)
{
    FILE *genmac;

    if (len != 6) {
        return FAILED;
    }

    genmac = fopen(WCNSS_GENMAC_FILE,"w");
    if (genmac == NULL) {
        ALOGE("%s: Failed to open %s: %s\n",
                __func__, WCNSS_GENMAC_FILE, strerror(errno));
        return FAILED;
    }
    fwrite(buf, 1, 6, genmac);
    fclose(genmac);

    return SUCCESS;
}

int wcnss_init_qmi(void)
{
    /* empty */
    return SUCCESS;
}

int wcnss_qmi_get_wlan_address(unsigned char *pBdAddr)
{
    int i, rc;
    unsigned char buf[6];
    int prefixlen = strnlen(WCNSS_INVALID_MAC_PREFIX, 8) / 2;

    // Use a previously stored value if it exists
    rc = read_mac(buf, 6);
    if (rc == SUCCESS) {
        ALOGI("%s: Got addr: %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
                __func__, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
        memcpy(pBdAddr, buf, 6);
        return SUCCESS;
    }

    rc = yl_get_wlan_address(buf, 6);
    if (rc != SUCCESS ||
            (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 &&
             buf[3] == 0 && buf[4] == 0 && buf[5] == 0)) {
        // Misconfigured device source...?
        if (prefixlen < 2) {
            return FAILED;
        }

        // Generate random MAC
        sscanf(WCNSS_INVALID_MAC_PREFIX, "%2hhx%2hhx%2hhx%2hhx",
                &buf[0], &buf[1], &buf[2], &buf[3]);

        // We don't need strong randomness, and if the NV is corrupted
        // any hardware values are suspect, so just seed it with the
        // current time
        srand(time(NULL));

        for (i = prefixlen; i < 6; i++) {
            buf[i] = rand() % 255;
        }

        ALOGI("%s: Generated addr: %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
                __func__, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    } else {
        ALOGI("%s: Got addr: %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
                __func__, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    }

    // Store for reuse
    write_mac(buf, 6);

    memcpy(pBdAddr, buf, 6);

    return SUCCESS;
}

void wcnss_qmi_deinit(void)
{
    /* empty */
}
