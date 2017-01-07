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

#include <iostream>
#include <fstream>
#include <string>

#include "vendor_init.h"
#include "property_service.h"
#include "log.h"
#include "util.h"

using namespace std;

typedef struct {
    string model;
    string description;
    string fingerprint;
    string default_network;
    bool is_cdma;
} match_t;

static match_t matches[] = {
    /* Honor 5x USA L24 (LTE, GSM/WCDMA) */
    {
        "KIW-L24",
        "KIW-L24-user 5.1.1 GRJ90 C567B140 release-keys",
        "HONOR/KIW-L24/HNKIW-Q:5.1.1/HONORKIW-L24/C567B140:user/release-keys",
        "9",
        false
    },
    /* Honor 5x Russia L23 (LTE, GSM/WCDMA) */
    {
        "KIW-L23",
        "KIW-L23-user 5.1.1 GRJ90 C567B140 release-keys",
        "HONOR/KIW-L23/HNKIW-Q:5.1.1/HONORKIW-L23/C567B140:user/release-keys",
        "9",
        false
    },
    /* Honor 5x India L22 (LTE, GSM/WCDMA) */
    {
        "KIW-L22",
        "KIW-L22-user 5.1.1 GRJ90 C675B130 release-keys",
        "HONOR/KIW-L22/HNKIW-Q:5.1.1/HONORKIW-L22/C675B130:user/release-keys",
        "9",
        false
    },
    /* Honor 5x EU L21 (LTE, GSM/WCDMA) */
    {
        "KIW-L21",
        "KIW-L21-user 5.1.1 GRJ90 C432B130 release-keys",
        "HONOR/KIW-L21/HNKIW-Q:5.1.1/HONORKIW-L21/C432B130:user/release-keys",
        "9",
        false
    },
    /* Honor 5x AL10 Chinese (TD-SCDMA/LTE/GSM/WCDMA, CDMA, and EvDo) */
    {
        "KIW-AL10",
        "KIW-AL10-user 5.1.1 GRJ90 C92B175 release-keys",
        "HONOR/KIW-AL10/HNKIW-Q:5.1.1/HONORKIW-AL10/C92B175:user/release-keys",
        "22",
        true
    },
    /* Honor 5x AL20 Chinese (TD-SCDMA/LTE/GSM/WCDMA, CDMA, and EvDo) */
    {
        "KIW-AL20",
        "KIW-AL20-user 5.1.1 GRJ90 C432B130 release-keys",
        "HONOR/KIW-AL20/HNKIW-Q:5.1.1/HONORKIW-AL20/C432B130:user/release-keys",
        "22",
        true
    },
    /* Chinese WCDMA/TD-SCDMA version KIW-UL00 (TD-SCDMA, GSM/WCDMA and LTE) */
    {
        "KIW-UL00",
        "KIW-UL00-user 5.1.1 GRJ90 C00B140 release-keys",
        "HONOR/KIW-UL00/HNKIW-Q:5.1.1/HONORKIW-UL00/C00B140:user/release-keys",
        "20",
        false
    },
    /* Chinese CDMA version KIW-CL00 (LTE, CDMA and EvDo) */
    {
        "KIW-CL00",
        "KIW-CL00-user 5.1.1 GRJ90 C92B140 release-keys",
        "HONOR/KIW-CL00/HNKIW-Q:5.1.1/HONORKIW-CL00/C92B140:user/release-keys",
        "8",
        true
    },
    /* Chinese TD-SCDMA version KIW-TL00H (TD-SCDMA,GSM and LTE) */
    {
        "KIW-TL00H",
        "KIW-TL00H-user 5.1.1 GRJ90 C00B140 release-keys",
        "HONOR/KIW-TL00H/HNKIW-Q:5.1.1/HONORKIW-TL00H/C00B140:user/release-keys",
        "17",
        false
    },
    /* Chinese TD-SCDMA version KIW-TL00 (TD-SCDMA,GSM and LTE) */
    {
        "KIW-TL00",
        "KIW-TL00-user 5.1.1 GRJ90 C00B140 release-keys",
        "HONOR/KIW-TL00/HNKIW-Q:5.1.1/HONORKIW-TL00/C00B140:user/release-keys",
        "17",
        false
    },
    /* HUAWEI GR5 version KII-L22 (same as honor, evidently from Japan) (LTE, GSM/WCDMA) */
    {
        "KII-L22",
        "KII-L22-user 5.1.1 GRJ90 C635B131 release-keys",
        "HUAWEI/KII-L22/HWKII-Q:5.1.1/HUAWEIKII-L22/C635B131:user/release-keys",
        "9",
        false
    },
    /* HUAWEI GR5 version KII-L21 (same as honor) (LTE, GSM/WCDMA) */
    {
        "KII-L21",
        "KII-L21-user 5.1.1 GRJ90 C185B130 release-keys",
        "HUAWEI/KII-L21/HWKII-Q:5.1.1/HUAWEIKII-L21/C185B130:user/release-keys",
        "9",
        false
    },
    /* HUAWEI GR5 version KII-L05 (same as honor, from Canada) */
    {
        "KII-L05",
        "KII-L05-user 6.0.1 GRJ90 C654B340 release-keys",
        "HUAWEI/KII-L05/HWKII-Q:6.0.1/HUAWEIKII-L05/C654B340:user/release-keys",
        "9",
        false
    },
};

static const int n_matches = sizeof(matches) / sizeof(matches[0]);

static int property_set(const char *key, string value)
{
    return property_set(key, value.c_str());
}

static bool contains(string str, string substr)
{
    return str.find(substr) != string::npos;
}

void vendor_load_properties()
{
    string platform;
    string model;
    string hwsim;
    match_t *match;

    platform = property_get("ro.board.platform");
    if (platform != ANDROID_TARGET)
        return;

    ifstream app_info("/proc/app_info");
    if (app_info.is_open()) {
        while (getline(app_info, model) && !contains(model, "huawei_fac_product_name")) {
        }
        app_info.close();
    }

    for (match = matches; match - matches < n_matches && !contains(model, match->model); match++) {
    }

    if (!match) {
        WARNING("Unknown variant: %s", model.c_str());
        return;
    }

    property_set("ro.build.product", "kiwi");
    property_set("ro.product.device", "kiwi");
    property_set("ro.product.model", match->model);
    property_set("ro.build.description", match->description);
    property_set("ro.build.fingerprint", match->fingerprint);
    if (match->is_cdma) {
        property_set("telephony.lteOnCdmaDevice", "1");
    }

    // Fix single sim variant based on property set by the bootloader
    hwsim = property_get("ro.boot.hwsim");

    if (hwsim == "single") {
        property_set("ro.telephony.default_network", match->default_network);
    } else {
        property_set("persist.radio.multisim.config", "dsds");
        property_set("ro.telephony.ril.config", "simactivation,sim2gsmonly");
        property_set("ro.telephony.default_network", match->default_network + "," +
                match->default_network);
    }
}
