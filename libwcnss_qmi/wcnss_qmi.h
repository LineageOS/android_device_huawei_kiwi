#ifndef _WCNSS_QMI_H_
#define _WCNSS_QMI_H_

#define RMT_OEMINFO_WIFI_MAC_ENC 0x34
#define MAC_SIZE 6

#define OEM_INFO_LIB_NAME "liboeminfo.so"
#define HUAWEI_SECURE_LIB_NAME "libhuawei_secure.so"

/* libhuawei_secure.so */
struct otp_key {
    uint8_t key1[0x104];
    uint8_t key2[0x104];
} __attribute__((packed));

#endif
