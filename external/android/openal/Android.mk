LOCAL_PATH	:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE 	:= 	openal
LOCAL_MODULE_FILENAME	:= libopenal
LOCAL_SRC_FILES	:= 	$(LOCAL_PATH)/$(TARGET_ARCH_ABI)/libopenal.so
LOCAL_EXPORT_C_INCLUDES	:= $(LOCAL_PATH)/include
include $(PREBUILT_SHARED_LIBRARY)
