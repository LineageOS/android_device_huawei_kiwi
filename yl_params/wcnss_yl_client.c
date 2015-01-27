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
#define WCNSS_INVALID_MAC_PREFIX "080000"
#endif

int wcnss_init_qmi(void)
{
    /* empty */
    return SUCCESS;
}

int wcnss_qmi_get_wlan_address(unsigned char *pBdAddr)
{
    int rc, i;
    char buf[6];
    struct stat statbuf;
    FILE *genmac;
    int prefixlen = strnlen(WCNSS_INVALID_MAC_PREFIX, 8) / 2;

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

    if (pBdAddr[0] == 0 && pBdAddr[1] == 0 && pBdAddr[2] == 0 &&
            pBdAddr[3] == 0 && pBdAddr[4] == 0 && pBdAddr[5] == 0) {

        // Misconfigured device source...?
        if (prefixlen < 2) {
            return FAILED;
        }

        // Use a previously stored value if it exists
        if (!stat(WCNSS_GENMAC_FILE, &statbuf)) {
            genmac = fopen(WCNSS_GENMAC_FILE,"r");
            if (fscanf(genmac, "%c%c%c%c%c%c", &pBdAddr[0],
                        &pBdAddr[1], &pBdAddr[2], &pBdAddr[3],
                        &pBdAddr[4], &pBdAddr[5]) == 6) {
                fclose(genmac);
                ALOGI("%s: Succesfully Read local WLAN MAC Address", __func__);
                return SUCCESS;
            }
            fclose(genmac);
        }

        sscanf(WCNSS_INVALID_MAC_PREFIX, "%2hhx%2hhx%2hhx%2hhx",
                &pBdAddr[0], &pBdAddr[1],
                &pBdAddr[2], &pBdAddr[3]);

        // We don't need strong randomness, and if the NV is corrupted
        // any hardware values are suspect, so just seed it with the
        // current time
        srand(time(NULL));

        for (i = prefixlen; i < 6; i++) {
            pBdAddr[i] = rand() % 255;
        }

        // Store for reuse
        genmac = fopen(WCNSS_GENMAC_FILE,"w");
        fwrite(pBdAddr, 1, 6, genmac);
        fclose(genmac);

        ALOGI("%s: Generated addr: %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
                __func__,
                pBdAddr[0], pBdAddr[1], pBdAddr[2],
                pBdAddr[3], pBdAddr[4], pBdAddr[5]);
    }

    return SUCCESS;
}

void wcnss_qmi_deinit(void)
{
    /* empty */
}
