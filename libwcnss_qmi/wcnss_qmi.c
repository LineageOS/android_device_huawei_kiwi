/*
 * Copyright (C) 2016  Rudolf Tammekivi <rtammekivi@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see [http://www.gnu.org/licenses/].
 */

#define LOG_TAG "libwcnss_qmi"
//#define LOG_NDEBUG 0

#include <dlfcn.h>
#include <cutils/log.h>
#include <string.h>

#include "wcnss_qmi.h"

// liboeminfo
int (*rmt_oeminfo_read)(int cell, int size, void *buf);

// libhuawei_secure
int (*crc_check)(void *key);
int (*otp_data_read)(void *key);
int (*rsa_public_decrypt)(void *buf, int size, void *dec_buf, int *dec_size, void *key);

void* oemApi;
void* huaweiSecure;

int wcnss_init_qmi(void)
{
    /* dlopen liboeminfo_oem_api and find the required functions */
    oemApi = dlopen(OEM_INFO_LIB_NAME, RTLD_NOW);
    if (!oemApi) {
        ALOGE("%s: failed to load %s: %s\n", __func__, OEM_INFO_LIB_NAME, dlerror());
        return -1;
    }

    rmt_oeminfo_read = dlsym(oemApi, "rmt_oeminfo_read");
    if (!rmt_oeminfo_read) {
        ALOGE("%s: failed to find rmt_oeminfo_read: %s\n", __func__, dlerror());
        goto close_oem_api;
    }

    /* dlopen libhuawei_secure and find the required functions */
    huaweiSecure = dlopen(HUAWEI_SECURE_LIB_NAME, RTLD_NOW);
    if (!huaweiSecure) {
        ALOGE("%s: failed to load %s: %s\n", __func__, HUAWEI_SECURE_LIB_NAME, dlerror());
        return -1;
    }

    crc_check = dlsym(huaweiSecure, "crc_check");
    if (!crc_check) {
        ALOGE("%s: failed to find crc_check: %s\n", __func__, dlerror());
        goto close_huawei_secure;
    }

    otp_data_read = dlsym(huaweiSecure, "otp_data_read");
    if (!otp_data_read) {
        ALOGE("%s: failed to find otp_data_read: %s\n", __func__, dlerror());
        goto close_huawei_secure;
    }

    rsa_public_decrypt = dlsym(huaweiSecure, "rsa_public_decrypt");
    if (!rsa_public_decrypt) {
        ALOGE("%s: failed to find rsa_public_decrypt: %s\n", __func__, dlerror());
        goto close_huawei_secure;
    }

    return 0;

close_huawei_secure:
    dlclose(huaweiSecure);
    huaweiSecure = NULL;

close_oem_api:
    dlclose(oemApi);
    oemApi = NULL;

    return -1;
}

void wcnss_qmi_deinit(void)
{
    if (oemApi) {
        dlclose(oemApi);
    }

    if (huaweiSecure) {
        dlclose(huaweiSecure);
    }
}

int wcnss_qmi_get_wlan_address(unsigned char *wlan_mac)
{
    struct otp_key key;

    uint8_t mac_crypted[0x80];
    uint8_t mac_decrypted[0xC];

    int mac_decrypted_size = 0;
    int ret = 0;
    int i;

    memset(&key, 0x0, sizeof(key));
    memset(mac_crypted, 0x0, sizeof(mac_crypted));
    memset(mac_decrypted, 0x0, sizeof(mac_decrypted));

    if (!rmt_oeminfo_read || !crc_check || !otp_data_read || !rsa_public_decrypt) {
        ALOGE("Failed earlier when finding functions, not reading MAC\n");
        return -1;
    }

    ALOGD("Reading RSA key\n");
    ret = otp_data_read(&key);
    if (ret) {
        printf("Failed to read RSA key ret=%d\n", ret);
        return ret;
    }

    ALOGD("Reading crypted Wi-Fi MAC address\n");
    ret = rmt_oeminfo_read(RMT_OEMINFO_WIFI_MAC_ENC, sizeof(mac_crypted),
                           mac_crypted);
    if (ret) {
        ALOGE("Failed to read crypted Wi-Fi MAC address ret=%d\n", ret);
        return -1;
    }

    ALOGD("Decrypting Wi-Fi MAC address\n");
    ret = rsa_public_decrypt(mac_crypted, sizeof(mac_crypted),
                             mac_decrypted, &mac_decrypted_size, key.key2);
    if (ret) {
        ALOGE("Failed to decrypt Wi-Fi MAC address ret=%d\n", ret);
        return ret;
    }

    if (mac_decrypted_size != sizeof(mac_decrypted)) {
        ALOGE("Failed to decrypt Wi-Fi MAC address mac_decrypted_size=%d\n",
              mac_decrypted_size);
        return -1;
    }

    ALOGD("Checking CRC of decrypted Wi-Fi MAC address\n");
    ret = crc_check(&mac_decrypted);
    if (ret) {
        ALOGE("Failed to check CRC of Wi-Fi MAC address ret=%d\n", ret);
        return -1;
    }

    /* MAC address is in reverse order. */
    for (i = 0; i < MAC_SIZE; i++)
        wlan_mac[i] = mac_decrypted[MAC_SIZE-1-i];

    return 0;
}
