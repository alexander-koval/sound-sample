//
//  SoundFile.cpp
//  HelloWorld
//
//  Created by Александр Коваль on 25.04.14.
//
//

#include <string>
#include <sndfile.h>
#include "SoundFile.h"
#include "ccMacros.h"
#include "platform/platform.h"
#include "platform/CCFileUtils.h"

using namespace cocos2d;

namespace cribys {
SoundFile* SoundFile::create(const char* pszFileName) {
    SoundFile* file = new SoundFile();
    if (file->initWithFile(pszFileName)) {
        file->autorelease();
        return file;
    }
    CC_SAFE_DELETE(file);
    return nullptr;
}

SoundFile::SoundFile()
: cocos2d::CCObject()
, m_file(nullptr)
, m_name(nullptr)
, m_sampleCount(0)
, m_channelCount(0)
, m_sampleRate(0)
, m_frames(0) {
}

SoundFile::~SoundFile() {
    CC_SAFE_DELETE_ARRAY(m_name);
    if (m_file) sf_close(m_file);
}

bool SoundFile::initWithFile(const char* pszFileName) {
    SF_INFO sound_info;
    SF_VIRTUAL_IO virtual_io;
    size_t len = strlen(pszFileName) + 1;
    char* name = new char[len];
    strcpy(name, pszFileName);
    name[len - 1] = '\0';
    m_name = name;
    unsigned long size = 0;
    std::string fullpath = CCFileUtils::sharedFileUtils()->fullPathForFilename(pszFileName);
    unsigned char* buffer = CCFileUtils::sharedFileUtils()->getFileData(fullpath.c_str(), "rb", &size);
    virtual_io.get_filelen = &SoundFile::getMemoryLength;
    virtual_io.read = &SoundFile::readMemory;
    virtual_io.seek = &SoundFile::seekMemory;
    virtual_io.tell = &SoundFile::tellMemory;
    virtual_io.write = &SoundFile::writeMemory;
    m_memory.dataStart = buffer;
    m_memory.dataPtr   = buffer;
    m_memory.totalSize = size;
    m_file = sf_open_virtual(&virtual_io, SFM_READ, &sound_info, &m_memory);
    if (m_file) {
        m_sampleCount  = static_cast<std::size_t>(sound_info.frames) * sound_info.channels;
        m_channelCount = sound_info.channels;
        m_sampleRate = sound_info.samplerate;
        m_frames = sound_info.frames;
        int format = sound_info.format & SF_FORMAT_TYPEMASK;
        int sub_format = sound_info.format & SF_FORMAT_SUBMASK;
        m_format.format = format;
        sf_command(m_file, SFC_GET_FORMAT_INFO, &m_format, sizeof(m_format));
        m_subFormat.format = sub_format;
        sf_command(m_file, SFC_GET_FORMAT_INFO, &m_subFormat, sizeof(m_subFormat));
        return true;
    } else {
        CCLOGERROR("Sound File can't be initialized: %s", sf_strerror(m_file));
        return false;
    }
}

std::size_t SoundFile::read(int16_t* data, std::size_t sampleCount) {
    if (m_file && data && sampleCount) {
        return static_cast<std::size_t>(sf_read_short(m_file, data, sampleCount));
    } else {
        return 0;
    }
}
    
void SoundFile::seek(float second) {
    if (m_file) {
        sf_count_t frameOffset = static_cast<sf_count_t>(second * m_sampleRate);
        sf_seek(m_file, frameOffset, SEEK_SET);
    }
}

const unsigned int SoundFile::getChannelCount(void) const {
    return m_channelCount;
}

const unsigned int SoundFile::getSampleRate(void) const {
    return m_sampleRate;
}

const std::size_t SoundFile::getSampleCount(void) const {
    return m_sampleCount;
}

const int64_t SoundFile::getFrames(void) const {
    return m_frames;
}

const SF_FORMAT_INFO& SoundFile::getFormat(void) const {
    return m_format;
}

const SF_FORMAT_INFO& SoundFile::getSubFormat(void) const {
    return m_subFormat;
}
    
const char* SoundFile::getName(void) const {
    return m_name;
}
    
sf_count_t SoundFile::getMemoryLength(void *user_data) {
    MemoryInfos* memory = static_cast<MemoryInfos*>(user_data);
    return memory->totalSize;
}
    
sf_count_t SoundFile::readMemory(void *ptr, sf_count_t count, void *user_data) {
    MemoryInfos* memory = static_cast<MemoryInfos*>(user_data);
    sf_count_t position = memory->dataPtr - memory->dataStart;
    if (position + count >= memory->totalSize)
        count = memory->totalSize - position;
    memcpy(ptr, memory->dataPtr, static_cast<std::size_t>(count));
    memory->dataPtr += count;
    return count;
}
    
sf_count_t SoundFile::seekMemory(sf_count_t offset, int whence, void *user_data) {
    MemoryInfos* memory = static_cast<MemoryInfos*>(user_data);
    sf_count_t position = 0;
    switch (whence) {
        case SEEK_SET :
            position = offset;
            break;
        case SEEK_CUR :
            position = memory->dataPtr - memory->dataStart + offset;
            break;
        case SEEK_END :
            position = memory->totalSize - offset;
            break;
        default :
            position = 0;
            break;
    }
    
    if (position >= memory->totalSize)
        position = memory->totalSize - 1;
    else if (position < 0)
        position = 0;
    memory->dataPtr = memory->dataStart + position;
    return position;
}
    
sf_count_t SoundFile::tellMemory(void *user_data) {
    MemoryInfos* memory = static_cast<MemoryInfos*>(user_data);
    return memory->dataPtr - memory->dataStart;
}
    
sf_count_t SoundFile::writeMemory(const void *ptr, sf_count_t count, void *user_data) {
    return 0;
}
    
unsigned long SoundFile::getMemorySize(void) const {
    return m_memory.totalSize;
}
    
}


