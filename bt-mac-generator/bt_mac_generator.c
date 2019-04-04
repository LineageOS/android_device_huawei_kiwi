/*
 * Copyright 2016, The Android Open Source Project
 * Copyright 2018, The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "BtMacGenerator"
// #define LOG_NDEBUG 0

#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#include <sys/stat.h>

/* liboeminfo.so */
#define RMT_OEMINFO_BT_MAC_ENC 0x33
extern int rmt_oeminfo_read(int cell, int size, void* buf);

static int force_random = 0;
static const char BDADDR_PATH[] = "/data/misc/bluedroid/qcomm_bda";

static void to_upper(char* str) {
    int i = 0;
    while (str[i] != '\0') {
        if ((str[i] >= 'a') && (str[i] <= 'z')) str[i] -= 32;
        i++;
    }
}

static void array2str(uint8_t* array, char* str) {
    int i;
    char c;
    for (i = 0; i < 6; i++) {
        c = (array[i] >> 4) & 0x0f;  // high 4 bit
        if (c >= 0 && c <= 9) {
            c += 0x30;
        } else if (c >= 0x0a && c <= 0x0f) {
            c = (c - 0x0a) + 'a' - 32;
        }
        *str++ = c;

        c = array[i] & 0x0f;  // low 4 bit
        if (c >= 0 && c <= 9) {
            c += 0x30;
        } else if (c >= 0x0a && c <= 0x0f) {
            c = (c - 0x0a) + 'a' - 32;
        }
        *str++ = c;

        if (i < 5) {
            *str++ = ':';
        }
    }
    *str = 0;
}

static int is_valid_mac_address(const char* pMacAddr) {
    int xdigit = 0;

    /* Mac full with zero */
    if (strcmp(pMacAddr, "000000000000") == 0) {
        return 0;
    }

    while (*pMacAddr) {
        if ((xdigit == 1) && ((*pMacAddr % 2) != 0)) break;

        if (isxdigit(*pMacAddr)) {
            xdigit++;
        }
        ++pMacAddr;
    }
    return (xdigit == 12 ? 1 : 0);
}

static void update_bt_mac(uint8_t* mac, bool random) {
    FILE* fb = NULL;
    struct stat st;
    int i = 0;
    uint8_t btMac[6];
    char bt_addr[20];

    memset(bt_addr, 0, 20);

    /* mac valid check */
    if (!random && mac != NULL) {
        for (i = 0; i < 6; i++) {
            btMac[i] = mac[6 - i - 1];
        }
        array2str(btMac, bt_addr);
        if (!is_valid_mac_address(bt_addr)) {  // invalid mac
            ALOGE("%s: Invalid mac, will generate random mac", __func__);
            random = true;
        }
    }

    if (random) {
        /* If file is exist and check its size or force reproduce it when first
         * start bt */
        if (force_random == 0) force_random++;

        /* If file is exist and check its size */
        if (force_random != 1 && stat(BDADDR_PATH, &st) == 0 && st.st_size >= 120) {
            ALOGD("%s: File %s already exists", __func__, BDADDR_PATH);
            return;
        } else {
            srand(time(NULL));
            memset(bt_addr, 0, 20);
            btMac[0] = 0xC0;
            btMac[1] = 0xEE;
            btMac[2] = 0xFB;
            btMac[3] = (rand() & 0x0FF00000) >> 20;
            btMac[4] = (rand() & 0x0FF00000) >> 20;
            btMac[5] = (rand() & 0x0FF00000) >> 20;
            array2str(btMac, bt_addr);

            if (force_random == 1) force_random++;
        }
    }

    ALOGV("%s: BT Addr: %s", __func__, bt_addr);

    if ((fb = fopen(BDADDR_PATH, "wb")) == NULL) {
        ALOGE("%s: Could not open bt mac file %s", __func__, BDADDR_PATH);
        return;
    }

    ALOGD("%s: Writing bt mac to file %s", __func__, BDADDR_PATH);
    fwrite(bt_addr, strlen(bt_addr), 1, fb);
    fclose(fb);

    chmod(BDADDR_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

    property_set("ro.bt.bdaddr_path", BDADDR_PATH);
    // Legacy
    property_set("persist.service.bdroid.bdaddr", BDADDR_PATH);
}

void get_mac_from_oeminfo() {
    uint8_t bt_addr[0x80];
    int ret = 0;
    int retries = 10;

    memset(bt_addr, 0, sizeof(bt_addr));

    // Wait till oeminfo successfully connect to socket
    while (retries-- > 0 && ret != 1) {
        ret = rmt_oeminfo_read(RMT_OEMINFO_BT_MAC_ENC, sizeof(bt_addr), bt_addr);
        ALOGD("%s: rmt_oeminfo_read result: ret = %d.(1 is success)", __func__, ret);
        usleep(1000000);
    }

    update_bt_mac(bt_addr, ret != 1);
}

int main() {
    get_mac_from_oeminfo();
    return 0;
}
