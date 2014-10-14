LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_SRC_FILES := $(LOCAL_PATH)/src/bitwise.c \
                   $(LOCAL_PATH)/src/framing.c
LOCAL_MODULE    := libogg
LOCAL_MODULE_TAGS := optional

LOCAL_SDK_VERSION := 19

include $(BUILD_STATIC_LIBRARY)
