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

#include <cutils/log.h>
#include <string.h>

/* liboeminfo_oem_api.so */
#define RMT_OEMINFO_WIFI_MAC_ENC 0x34
extern int remote_oeminfo_read(int cell, int size, void *buf);

/* libhuawei_secure.so */
struct otp_key {
    uint8_t key1[0x104];
    uint8_t key2[0x104];
} __attribute__((packed));

/* Read RSA public key */
extern int otp_data_read(void *key);

extern int rsa_public_decrypt(void *buf, int size, void *dec_buf, int *dec_size,
                              void *key);

#define MAC_SIZE 6

int wcnss_init_qmi(void)
{
    return 0;
}

void wcnss_qmi_deinit(void)
{
}

int wcnss_qmi_get_wlan_address(unsigned char *wlan_mac)
{
    struct otp_key key;

    uint8_t mac_crypted[0x80];
    uint8_t mac_decrypted[0xC];

    int mac_decrypted_size = 0;
    int ret = 0;
    int i;

    memset(mac_decrypted, 0x0, sizeof(mac_decrypted));

    ALOGD("Reading RSA key\n");
    ret = otp_data_read(&key);
    if (ret) {
        printf("Failed to read RSA key ret=%d\n", ret);
        return ret;
    }

    ALOGD("Reading crypted Wi-Fi MAC address\n");
    ret = remote_oeminfo_read(RMT_OEMINFO_WIFI_MAC_ENC,
                  sizeof(mac_crypted), mac_crypted);
    if (ret != 1) {
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

    /* MAC address is in reverse order. */
    for (i = 0; i < MAC_SIZE; i++)
        wlan_mac[i] = mac_decrypted[MAC_SIZE-1-i];

    return 0;
}
