LOCAL_PATH	:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE	:=	ogg_static
LOCAL_SRC_FILES	:=	$(LOCAL_PATH)/ogg/$(TARGET_ARCH_ABI)/libogg.a
LOCAL_EXPORT_C_INCLUDES	:= $(LOCAL_PATH)/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:= 	vorbis_static 
LOCAL_SRC_FILES	:=	$(LOCAL_PATH)/vorbis/$(TARGET_ARCH_ABI)/libvorbis.a 
LOCAL_EXPORT_C_INCLUDES	:= $(LOCAL_PATH)/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:=	vorbisenc_static
LOCAL_SRC_FILES	:=	$(LOCAL_PATH)/vorbis/$(TARGET_ARCH_ABI)/libvorbisenc.a
LOCAL_EXPORT_C_INCLUDES	:= $(LOCAL_PATH)/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:= 	vorbisfile_static
LOCAL_SRC_FILES	:=	$(LOCAL_PATH)/vorbis/$(TARGET_ARCH_ABI)/libvorbisfile.a
LOCAL_EXPORT_C_INCLUDES	:= $(LOCAL_PATH)/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:=	FLAC
LOCAL_SRC_FILES	:=	$(LOCAL_PATH)/flac/$(TARGET_ARCH_ABI)/libFLAC.so
LOCAL_EXPORT_C_INCLUDES	:= $(LOCAL_PATH)/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE 	:= 	sndfile
LOCAL_SRC_FILES	:= 	$(LOCAL_PATH)/sndfile/$(TARGET_ARCH_ABI)/libsndfile.so
LOCAL_EXPORT_C_INCLUDES	:= $(LOCAL_PATH)/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE 	:= 	openal
LOCAL_SRC_FILES	:= 	$(LOCAL_PATH)/openal/$(TARGET_ARCH_ABI)/libopenal.so
LOCAL_EXPORT_C_INCLUDES	:= $(LOCAL_PATH)/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS) 
LOCAL_MODULE := soundpack
LOCAL_WHOLE_STATIC_LIBRARIES := ogg_static vorbis_static vorbisenc_static vorbisfile_static
LOCAL_WHOLE_SHARED_LIBRARIES := FLAC sndfile openal
LOCAL_LDLIBS := -Wl,--whole-archive vorbis_static vorbisenc_static vorbisfile_static FLAC sndfile openal
include $(BUILD_SHARED_LIBRARY)
