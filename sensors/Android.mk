LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sensors.$(TARGET_BOARD_PLATFORM)

LOCAL_MODULE_RELATIVE_PATH    := hw

LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES :=	\
        sensors.cpp             \
        SensorBase.cpp          \
        LightSensor.cpp         \
        ProximitySensor.cpp     \
        Accelerometer.cpp       \
        Gyroscope.cpp           \
        InputEventReader.cpp    \
        CalibrationManager.cpp  \
		TomatoSensors.cpp       \
		NativeSensorManager.cpp \
		VirtualSensor.cpp 

LOCAL_SHARED_LIBRARIES := liblog libcutils libdl libutils

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := libcalmodule_common
LOCAL_SRC_FILES := \
           algo/common/common_wrapper.c \
           algo/common/compass/AKFS_AOC.c \
           algo/common/compass/AKFS_Device.c \
           algo/common/compass/AKFS_Direction.c \
           algo/common/compass/AKFS_VNorm.c

LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_MODULE_TAGS := optional

ifdef TARGET_2ND_ARCH
LOCAL_MODULE_PATH_32 := $(TARGET_OUT_VENDOR)/lib
LOCAL_MODULE_PATH_64 := $(TARGET_OUT_VENDOR)/lib64
else
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)
endif

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := calmodule.cfg
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES := calmodule.cfg

include $(BUILD_PREBUILT)
