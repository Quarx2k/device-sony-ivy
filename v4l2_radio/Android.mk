LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := radio_v4l
LOCAL_SRC_FILES := radio_v4l.c
LOCAL_SHARED_LIBRARIES := libc libcutils libutils liblog libtinyalsa
LOCAL_C_INCLUDES := \
	external/tinyalsa/include \
	hardware/libhardware/include

include $(BUILD_EXECUTABLE)
