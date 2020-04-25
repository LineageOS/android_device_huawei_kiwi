LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \
	libdl liblog libutils libcutils libtinyalsa

LOCAL_HEADER_LIBRARIES := generated_kernel_headers

LOCAL_C_INCLUDES := \
	$(call project-path-for,qcom-audio)/hal/ \
	external/tinyalsa/include \
	hardware/libhardware/include

LOCAL_SRC_FILES := \
	audio_amplifier.c

LOCAL_MODULE := audio_amplifier.msm8916
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS = -Werror

include $(BUILD_SHARED_LIBRARY)
