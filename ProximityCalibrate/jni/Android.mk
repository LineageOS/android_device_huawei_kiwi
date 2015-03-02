LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := jni_proximityCalibrate.c
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libjni_proximityCalibrate
LOCAL_SHARED_LIBRARIES := libcutils liblog
include $(BUILD_SHARED_LIBRARY)
