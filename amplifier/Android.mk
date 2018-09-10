LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \
	liblog libutils libcutils libtfa9895 libtinyalsa

LOCAL_C_INCLUDES := \
	$(call project-path-for,qcom-audio)/hal/ \
	$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
	external/tinyalsa/include \
	hardware/libhardware/include

LOCAL_ADDITIONAL_DEPENDENCIES += \
	$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES := \
	audio_amplifier.c

LOCAL_MODULE := audio_amplifier.msm8916
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS = -Werror

include $(BUILD_SHARED_LIBRARY)
