LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    exif.c \
    get_app_info.c \
    gps.c \
    log.c \
    widevine.c

LOCAL_SHARED_LIBRARIES := \

LOCAL_MODULE:= libcm_kiwi_preload
include $(BUILD_SHARED_LIBRARY)
