LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

src_proto := $(LOCAL_PATH)
LOCAL_MODULE := sap-api-java-static
LOCAL_SRC_FILES := sap-api.proto
LOCAL_PROTOC_OPTIMIZE_TYPE := micro

include $(BUILD_STATIC_JAVA_LIBRARY)

