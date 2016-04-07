LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := device/qcom/common/power
LOCAL_CFLAGS := -Wall -Werror
LOCAL_SRC_FILES := power-kiwi.c
LOCAL_MODULE := libpower_kiwi
include $(BUILD_STATIC_LIBRARY)
