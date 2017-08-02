ifeq ($(WITH_TWRP),true)
TARGET_RECOVERY_DEVICE_DIRS += device/huawei/kiwi/twrp
TARGET_RECOVERY_PIXEL_FORMAT := RGBX_8888
TW_INCLUDE_CRYPTO := true
TW_INPUT_BLACKLIST := "accelerometer\x0ahbtp_vm"
TW_THEME := portrait_hdpi
endif
