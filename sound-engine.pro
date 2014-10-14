QT       -= core gui
TARGET = soundengine
TEMPLATE = lib
#CONFIG += sharedlib
CONFIG += staticlib
CONFIG -= qt

CCFLAGS += -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wno-return-type -Wno-ignored-qualifiers -Wno-type-limits -fPIC -MMD
QMAKE_CFLAGS +=  $$CCFLAGS -Wno-implicit-function-declaration -Wno-reorder
QMAKE_CXXFLAGS += $$CCFLAGS -std=c++11
QMAKE_CFLAGS_DEBUG += $$QMAKE_CFLAGS -g3 -O0
QMAKE_CXXFLAGS_DEBUG += $$QMAKE_CXXFLAGS -g3 -O0

QMAKE_LFLAGS += -Wl,--no-as-needed

debug {
    DEFINES += DEBUG COCOS2D_DEBUG=1
}

ENGINE_INCLUDE_PATH += \
    Audio \
    ../cocos2dx/cocos2dx \
    ../cocos2dx/cocos2dx/include \
    ../cocos2dx/cocos2dx/kazmath/include

SOURCES += \
    Audio/Audio.cpp \
    Audio/Sound.cpp \
    Audio/SoundBuffer.cpp \
    Audio/SoundFile.cpp \
    Audio/SoundCache.cpp

HEADERS += \
    Audio/Audio.h \
    Audio/Sound.h \
    Audio/SoundBuffer.h \
    Audio/SoundFile.h \
    Audio/SoundCache.h

unix:!mac:!android {
    DEFINES += LINUX
    ENGINE_INCLUDE_PATH += \
        /usr/lib \
        /usr/local/lib \
        $$_PRO_FILE_PWD_/../cocos2dx/cocos2dx/platform/linux \
        $$_PRO_FILE_PWD_/../cocos2dx/cocos2dx/platform/third_party/linux/libjpeg \
        $$_PRO_FILE_PWD_/../cocos2dx/cocos2dx/platform/third_party/linux/libpng \
        $$_PRO_FILE_PWD_/../cocos2dx/cocos2dx/platform/third_party/linux/libtiff/include \
        $$_PRO_FILE_PWD_/../cocos2dx/cocos2dx/platform/third_party/linux/libwebp
}

android {

}

LIBS += -lcocos2dx -lCocosDenshion -lopenal -lsndfile

INCLUDEPATH += $${ENGINE_INCLUDE_PATH}
