LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(call project-path-for,recovery)
LOCAL_SRC_FILES := recovery_updater.cpp
LOCAL_MODULE := librecovery_updater_kiwi
LOCAL_MODULE_TAGS := eng
include $(BUILD_STATIC_LIBRARY)
