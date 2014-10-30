LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sensors.$(TARGET_BOARD_PLATFORM)

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

LOCAL_SRC_FILES :=	\
        sensors.cpp             \
        SensorBase.cpp          \
        LightSensor.cpp         \
        ProximitySensor.cpp     \
        Accelerometer.cpp       \
        Gyroscope.cpp           \
        InputEventReader.cpp    \
        CalibrationManager.cpp

LOCAL_SHARED_LIBRARIES := liblog libcutils libdl

include $(BUILD_SHARED_LIBRARY)

