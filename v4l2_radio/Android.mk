LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libfmradio.v4l2-fm

LOCAL_SRC_FILES := v4l2_fm.c \
	           v4l2_ioctl.c \
	           v4l2_ioctl.h

LOCAL_SHARED_LIBRARIES := libc \
                          liblog

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := radio_v4l
LOCAL_SRC_FILES := radio_v4l.c
LOCAL_SHARED_LIBRARIES := libc libtinyalsa libfmradio.v4l2-fm
LOCAL_C_INCLUDES := \
	external/tinyalsa/include \
	hardware/libhardware/include

include $(BUILD_EXECUTABLE)

include $(LOCAL_PATH)/broadcom_fm/Android.mk
