//
//  SoundBuffer.cpp
//  HelloWorld
//
//  Created by Александр Коваль on 25.04.14.
//
//

#include "SoundBuffer.h"
#include "ccMacros.h"
#include "Audio.h"
#include "SoundFile.h"
#include "Sound.h"
#include "cocoa/CCZone.h"
#include "platform/CCCommon.h"

namespace cribys {
    
SoundBuffer* SoundBuffer::create(void) {
    SoundBuffer* buffer = new SoundBuffer();
    buffer->autorelease();
    return buffer;
}
    
SoundBuffer* SoundBuffer::create(const char* pszFileName) {
    SoundBuffer* buffer = new SoundBuffer();
    if (buffer->initWithFile(pszFileName)) {
        buffer->autorelease();
        return buffer;
    }
    CC_SAFE_DELETE(buffer);
    return nullptr;
}

SoundBuffer* SoundBuffer::create(SoundFile *file) {
    SoundBuffer* buffer = new SoundBuffer();
    if (file && buffer->initWithFile(file)) {
        buffer->autorelease();
        return buffer;
    }
    CC_SAFE_DELETE(buffer);
    return nullptr;
}

SoundBuffer::SoundBuffer()
: cocos2d::CCObject()
, m_duration(0)
, m_buffer(0)
, m_memorySize(0) {
    AL_CHECK_ERRORS(alGenBuffers(1, &m_buffer));
}

SoundBuffer::~SoundBuffer() {
    for (std::set<Sound*>::iterator it = m_sounds.begin(); it != m_sounds.end(); ++it) {
        (*it)->resetBuffer();
    }
    if (m_buffer) AL_CHECK_ERRORS((alDeleteBuffers(1, &m_buffer)));
}
    
cocos2d::CCObject* SoundBuffer::copyWithZone(cocos2d::CCZone* zone) {
    cocos2d::CCZone* new_zone = nullptr;
    SoundBuffer* copy = nullptr;
    if (zone && zone->m_pCopyObject) {
        copy = static_cast<SoundBuffer*>(zone->m_pCopyObject);
    } else {
        copy = SoundBuffer::create();
        zone = new_zone = new cocos2d::CCZone(copy);
    }
    copy->m_samples = m_samples;
    copy->initialize(this->getChannelCount(), this->getSampleRate());
    CC_SAFE_DELETE(new_zone);
    return copy;
}

bool SoundBuffer::initWithFile(const char* pszFileName) {
    SoundFile* file = SoundFile::create(pszFileName);
    if (!file) return false;
    return this->initWithFile(file);
}

bool SoundBuffer::initWithFile(SoundFile *file) {
    file->retain();
    std::size_t  sample_count  = file->getSampleCount();
    unsigned int channel_count = file->getChannelCount();
    unsigned int sample_rate   = file->getSampleRate();
    CCLOG("SOUND: %s, sample_count %ld, channel_count %u, sample_rate %u", file->getName(), file->getSampleCount(), file->getChannelCount(), file->getSampleRate());
    m_samples.resize(sample_count);
    size_t size = file->read(&m_samples[0], sample_count);
    if (size != sample_count) {
        CCLOGERROR("samples count mistmatch");
        m_samples.resize(size);
        sample_count = size;
    }
    m_memorySize = file->getMemorySize();
    file->release();
    return initialize(channel_count, sample_rate);
}

bool SoundBuffer::initialize(unsigned int channel_count, unsigned int sample_rate) {
    if (!channel_count || !sample_rate || m_samples.empty()) return false;
    ALenum format = Audio::getFormatFromChannelCount(channel_count);
    if (format == 0) {
        CCLOGERROR("unsupported number of channels %d", channel_count);
        return false;
    }
    ALsizei size = static_cast<ALsizei>(m_samples.size()) * sizeof(int16_t);
    AL_CHECK_ERRORS(alBufferData(m_buffer, format, &m_samples[0], size, sample_rate));
    m_duration = AudioTime(static_cast<float>(m_samples.size()) / sample_rate / channel_count);
    return true;
}

const int16_t* SoundBuffer::getSamples(void) const {
    return m_samples.empty() ? nullptr : &m_samples[0];
}

std::size_t SoundBuffer::getSampleCount(void) const {
    return m_samples.size();
}

unsigned int SoundBuffer::getSampleRate(void) const {
    ALint sample_rate;
    AL_CHECK_ERRORS((alGetBufferi(m_buffer, AL_FREQUENCY, &sample_rate)));
    return sample_rate;
}

unsigned int SoundBuffer::getChannelCount(void) const {
    ALint channel_count;
    AL_CHECK_ERRORS((alGetBufferi(m_buffer, AL_CHANNELS, &channel_count)));
    return channel_count;
}
    
unsigned long SoundBuffer::getFileSize(void) const {
    return m_memorySize;
}

void SoundBuffer::attachSound(Sound* sound) {
    m_sounds.insert(sound);
}

void SoundBuffer::detachSound(Sound* sound) {
    m_sounds.erase(sound);
}

AudioTime SoundBuffer::getDuration(void) const {
    return m_duration;
}
}
