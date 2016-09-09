LOCAL_PATH:= $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_DEX_PREOPT := false

LOCAL_SRC_FILES := $(call all-java-files-under, src/com/stericsson/hardware/fm) \
        src/com/stericsson/hardware/fm/IFmReceiver.aidl \
        src/com/stericsson/hardware/fm/IFmTransmitter.aidl \
        src/com/stericsson/hardware/fm/IOnAutomaticSwitchListener.aidl \
        src/com/stericsson/hardware/fm/IOnBlockScanListener.aidl \
        src/com/stericsson/hardware/fm/IOnErrorListener.aidl \
        src/com/stericsson/hardware/fm/IOnExtraCommandListener.aidl \
        src/com/stericsson/hardware/fm/IOnForcedPauseListener.aidl \
        src/com/stericsson/hardware/fm/IOnForcedResetListener.aidl \
        src/com/stericsson/hardware/fm/IOnRDSDataFoundListener.aidl \
        src/com/stericsson/hardware/fm/IOnScanListener.aidl \
        src/com/stericsson/hardware/fm/IOnSignalStrengthListener.aidl \
        src/com/stericsson/hardware/fm/IOnStartedListener.aidl \
        src/com/stericsson/hardware/fm/IOnStateChangedListener.aidl \
        src/com/stericsson/hardware/fm/IOnStereoListener.aidl 

LOCAL_JNI_SHARED_LIBRARIES := libbroadcomfm_jni

LOCAL_MODULE:= broadcom.fmradio

include $(BUILD_JAVA_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        libbroadcomfm/android_fmradio.cpp \
        libbroadcomfm/android_fmradio_Receiver.cpp \
        libbroadcomfm/android_fmradio_Transmitter.cpp

LOCAL_C_INCLUDES += \
        $(JNI_H_INCLUDE)\
        $(TOP)/frameworks/base/fmradio/include \
        $(TOP)/hardware/libhardware/include/hardware \
        $(LOCAL_PATH)/../include

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        libhardware \
        libhardware_legacy \
        libnativehelper \
        libutils \

ifeq ($(TARGET_SIMULATOR),true)
ifeq ($(TARGET_OS),linux)
ifeq ($(TARGET_ARCH),x86)
LOCAL_LDLIBS += -lpthread -ldl -lrt
endif
endif
endif

ifeq ($(WITH_MALLOC_LEAK_CHECK),true)
        LOCAL_CFLAGS += -DMALLOC_LEAK_CHECK
endif

LOCAL_MODULE:= libbroadcomfm
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
