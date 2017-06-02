LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS := -DLOG_TAG=\"VRNative-JNI\"

LOCAL_C_INCLUDES += $(LOCAL_PATH)/binder

LOCAL_SRC_FILES:= \
    binder/IVRPostEvent.cpp \
    binder/IVRNativeService.cpp \
    VRNativeJNI.cpp

LOCAL_SHARED_LIBRARIES := \
    libandroid_runtime \
    libnativehelper \
    libbinder \
    libcutils \
    liblog \
    libutils

LOCAL_CFLAGS += -Wall # -Werror -Wno-error=deprecated-declarations -Wunused -Wunreachable-code

LOCAL_MODULE:= libvrnative_jni

include $(BUILD_SHARED_LIBRARY)

# VR natvie service
include $(CLEAR_VARS)

LOCAL_CFLAGS := -DLOG_TAG=\"VRNatvieServer\"
LOCAL_CFLAGS += -DVRS_DEBUG=1
LOCAL_CFLAGS += -DVRS_BUILDTIME=\"$(shell date +%Y%m%d%H%M%S)\"

LOCAL_C_INCLUDES += $(LOCAL_PATH)/binder \
                    external/libxml2/include \
                    external/icu/icu4c/source/common

LOCAL_SRC_FILES :=  \
    binder/IVRPostEvent.cpp \
    binder/IVRNativeService.cpp \
    main/Thread.cpp \
    main/Connector.cpp \
    main/ClientConnector.cpp \
    main/ServerListener.cpp \
    main/VrSDK.cpp \
    main/XmlDatabase.cpp \
    main/VRNativeService.cpp \
    main/main_vrserver.cpp

LOCAL_SHARED_LIBRARIES := \
    libandroid \
    libbinder \
    libcutils \
    liblog \
    libutils \
    libxml2

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := vrserver

include $(BUILD_EXECUTABLE)

include $(LOCAL_PATH)/prebuilt/Android.mk
