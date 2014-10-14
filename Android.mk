LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

COCOS2DX_ROOT 		:= $(abspath $(LOCAL_PATH)/../cocos2dx)

LOCAL_MODULE	:=	soundengine
LOCAL_MODULE_FILENAME	:= libsoundengine
LOCAL_SRC_FILES	:=	$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/Audio/*.cpp)) \
					$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/actions/*.cpp))
LOCAL_C_INCLUDES 		:= 	$(LOCAL_PATH) \
							$(LOCAL_PATH)/Audio \
							$(LOCAL_PATH)/external/include \
							$(LOCAL_PATH)/external/android/include
LOCAL_EXPORT_C_INCLUDES := 	$(LOCAL_PATH) \
							$(LOCAL_PATH)/Audio \
							$(LOCAL_PATH)/external/include \
							$(LOCAL_PATH)/external/android/include
					
LOCAL_WHOLE_STATIC_LIBRARIES 	:= ogg vorbis cocos2dx_static
LOCAL_WHOLE_SHARED_LIBRARIES	:= openal sndfile FLAC
LOCAL_SHARED_LIBRARIES			:= openal sndfile FLAC

#LOCAL_LDLIBS					:= -lopenal -lFLAC -lsndfile
#LOCAL_EXPORT_LDLIBS				:= -lopenal	-lFLAC -lsndfile

include $(BUILD_STATIC_LIBRARY)
$(call import-add-path,$(COCOS2DX_ROOT))
$(call import-add-path,$(LOCAL_PATH)/external/android/libs)
$(call import-module,cocos2dx)
$(call import-module,openal)
$(call import-module,ogg)
$(call import-module,vorbis)
$(call import-module,flac)
$(call import-module,sndfile)
