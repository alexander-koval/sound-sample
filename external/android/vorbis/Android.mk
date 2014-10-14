LOCAL_PATH	:=	$(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE	:= 	vorbis_static
LOCAL_SRC_FILES	:=	$(LOCAL_PATH)/$(TARGET_ARCH_ABI)/libvorbis.a 
LOCAL_EXPORT_C_INCLUDES	:= $(LOCAL_PATH)/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:=	vorbisenc_static
LOCAL_SRC_FILES	:=	$(LOCAL_PATH)/$(TARGET_ARCH_ABI)/libvorbisenc.a
LOCAL_EXPORT_C_INCLUDES	:= $(LOCAL_PATH)/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:= 	vorbisfile_static
LOCAL_SRC_FILES	:=	$(LOCAL_PATH)/$(TARGET_ARCH_ABI)/libvorbisfile.a
LOCAL_EXPORT_C_INCLUDES	:= $(LOCAL_PATH)/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS) 
LOCAL_MODULE := vorbis
LOCAL_WHOLE_STATIC_LIBRARIES := vorbis_static vorbisenc_static vorbisfile_static 
#LOCAL_LDLIBS := -Wl,--whole-archive vorbis_static vorbisenc_static vorbisfile_static
include $(BUILD_STATIC_LIBRARY) 
