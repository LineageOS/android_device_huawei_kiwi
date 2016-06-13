/*
   Copyright (c) 2016, The Linux Foundation. All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vendor_init.h"
#include "property_service.h"
#include "log.h"
#include "util.h"

typedef struct {
    const char *model;
    const char *description;
    const char *fingerprint;
} match_t;

static match_t matches[] = {
    /* Honor 5x USA L24 */
    {
        "KIW-L24",
        "KIW-L24-user 5.1.1 GRJ90 C567B140 release-keys",
        "HONOR/KIW-L24/HNKIW-Q:5.1.1/HONORKIW-L24/C567B140:user/release-keys"
    },
    /* Honor 5x Russia L23 */
    {
        "KIW-L23",
        "KIW-L23-user 5.1.1 GRJ90 C567B140 release-keys",
        "HONOR/KIW-L23/HNKIW-Q:5.1.1/HONORKIW-L23/C567B140:user/release-keys"
    },
    /* Honor 5x India L22 */
    {
        "KIW-L22",
        "KIW-L22-user 5.1.1 GRJ90 C675B130 release-keys",
        "HONOR/KIW-L22/HNKIW-Q:5.1.1/HONORKIW-L22/C675B130:user/release-keys"
    },
    /* Honor 5x EU L21 */
    {
        "KIW-L21",
        "KIW-L21-user 5.1.1 GRJ90 C432B130 release-keys",
        "HONOR/KIW-L21/HNKIW-Q:5.1.1/HONORKIW-L21/C432B130:user/release-keys"
    },
    /* Honor 5x AL10 Chinese */
    {
        "KIW-AL10",
        "KIW-AL10-user 5.1.1 GRJ90 C92B175 release-keys",
        "HONOR/KIW-AL10/HNKIW-Q:5.1.1/HONORKIW-AL10/C92B175:user/release-keys"
    },
    /* Honor 5x AL20 Chinese */
    {
        "KIW-AL20",
        "KIW-AL20-user 5.1.1 GRJ90 C432B130 release-keys",
        "HONOR/KIW-AL20/HNKIW-Q:5.1.1/HONORKIW-AL20/C432B130:user/release-keys"
    },
    /* Chinese WCDMA version KIW-UL00 */
    {
        "KIW-UL00",
        "KIW-UL00-user 5.1.1 GRJ90 C00B140 release-keys",
        "HONOR/KIW-UL00/HNKIW-Q:5.1.1/HONORKIW-UL00/C00B140:user/release-keys"
    },
    /* HUAWEI GX5 version KII-L22 (same as honor, evidently from Japan) */
    {
        "KII-L22",
        "KII-L22-user 5.1.1 GRJ90 C635B131 release-keys",
        "HUAWEI/KII-L22/HWKII-Q:5.1.1/HUAWEIKII-L22/C635B131:user/release-keys"
    },
    /* HUAWEI GX5 version KII-L21 (same as honor) */
    {
        "KII-L21",
        "KII-L21-user 5.1.1 GRJ90 C185B130 release-keys",
        "HUAWEI/KII-L21/HWKII-Q:5.1.1/HUAWEIKII-L21/C185B130:user/release-keys"
    },
    { 0, 0, 0 }
};

void vendor_load_properties()
{
    char platform[PROP_VALUE_MAX];
    char model[110];
    char hwsim[PROP_VALUE_MAX];
    FILE* fp;
    int rc;
    match_t *match;

    rc = property_get("ro.board.platform", platform);
    if (!rc || strncmp(platform, ANDROID_TARGET, PROP_VALUE_MAX))
        return;

    fp = fopen("/proc/app_info", "rb");
    while (fgets(model, 100, fp))
        if (strstr(model, "huawei_fac_product_name") != NULL)
            break;

    for (match = matches; match->model; match++) {
        if (strstr(model, match->model)) {
            property_set("ro.build.product", "kiwi");
            property_set("ro.product.device", "kiwi");
            property_set("ro.product.model", match->model);
            property_set("ro.build.description", match->description);
            property_set("ro.build.fingerprint", match->fingerprint);
            break;
        }
    }

    // Fix single sim variant based on property set by the bootloader
    rc = property_get("ro.boot.hwsim", hwsim);

    if (rc > 0 && !strncmp(hwsim, "single", PROP_VALUE_MAX)) {
        property_set("ro.telephony.default_network", "9");
    } else {
        property_set("persist.radio.multisim.config", "dsds");
        property_set("ro.telephony.ril.config", "simactivation,sim2gsmonly");
        property_set("ro.telephony.default_network", "9,9");
    }
}
