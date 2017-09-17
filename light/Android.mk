LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := android.hardware.light@2.0-service.kiwi
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_INIT_RC = android.hardware.light@2.0-service.kiwi.rc

LOCAL_SRC_FILES := \
    service.cpp \
    Light.cpp

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libhardware \
    libhidlbase \
    libhidltransport \
    liblog \
    libhwbinder \
    libutils \
    android.hardware.light@2.0 \
    libqmi_oem_api

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)
