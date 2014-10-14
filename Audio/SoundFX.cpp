//
//  SoundFX.cpp
//  CribysUtils
//
//  Created by Александр Коваль on 04.06.14.
//  Copyright (c) 2014 Cribys. All rights reserved.
//

#include "SoundFX.h"
#include "SoundFile.h"
#include "cocoa/CCZone.h"

namespace cribys {

SoundFX* SoundFX::create(void) {
    SoundFX* sound_fx = new SoundFX();
    sound_fx->autorelease();
    return sound_fx;
}
    
SoundFX* SoundFX::create(const char *pszFileName) {
    SoundFX* sound_fx = new SoundFX();
    if (sound_fx->initWithFile(pszFileName)) {
        sound_fx->autorelease();
        return sound_fx;
    }
    CC_SAFE_DELETE(sound_fx);
    return nullptr;
}
    
SoundFX* SoundFX::create(cribys::SoundFile *file) {
    SoundFX* sound_fx = new SoundFX();
    if (sound_fx->initWithFile(file)) {
        sound_fx->autorelease();
        return sound_fx;
    }
    CC_SAFE_DELETE(sound_fx);
    return nullptr;
}
    
SoundFX::SoundFX(void)
: SoundStream()
, m_file(nullptr) {
        
}
    
SoundFX::~SoundFX(void) {
    CC_SAFE_DELETE_ARRAY(m_fileName);
    CC_SAFE_RELEASE(m_file);
}
    
bool SoundFX::initWithFile(const char *pszFileName) {
    SoundFile* file = SoundFile::create(pszFileName);
    return this->initWithFile(file);
}
    
bool SoundFX::initWithFile(cribys::SoundFile *file) {
    m_file = file;
    if (!m_file) return false;
    unsigned int channel_count = file->getChannelCount();
    unsigned int sample_rate   = file->getSampleRate();
    size_t len = strlen(file->getName()) + 1;
    char* name = new char[len];
    strcpy(name, file->getName());
    name[len - 1] = '\0';
    m_fileName = name;
    CCLOG("SOUND_FX: %s, sample_count %ld, channel_count %u, sample_rate %u", file->getName(), file->getSampleCount(), file->getChannelCount(), file->getSampleRate());
    m_file->retain();
    return this->initialize(channel_count, sample_rate);
}
    
void SoundFX::stop(void) {
    SoundStream::stop();
    this->seek(0.0f);
}
    
void SoundFX::seek(float second) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_file->seek(second);
}
    
cocos2d::CCObject* SoundFX::copyWithZone(cocos2d::CCZone *zone) {
    cocos2d::CCZone* new_zone = nullptr;
    SoundFX* copy = nullptr;
    if (zone && zone->m_pCopyObject) {
        copy = static_cast<SoundFX*>(zone->m_pCopyObject);
    } else {
        size_t len = strlen(this->getFileName()) + 1;
        char* filename = new char[len];
        strcpy(filename, this->getFileName());
        filename[len - 1] = '\0';
        copy = SoundFX::create(filename);
        zone = new_zone = new cocos2d::CCZone(copy);
    }
    copy->m_samples = m_samples;
    copy->initialize(this->getChannelCount(), this->getSampleRate());
    CC_SAFE_DELETE(new_zone);
    return copy;
}
    
const int16_t* SoundFX::getSamples(void) const {
    return m_samples.empty() ? nullptr : &m_samples[0];
}
    
std::size_t SoundFX::getSampleCount(void) const {
    return m_samples.size();
}
    
unsigned long SoundFX::getFileSize(void) const {
    return m_file->getMemorySize();
}

bool SoundFX::initialize(unsigned int channel_count, unsigned int sample_rate) {
    m_duration = AudioTime(static_cast<float>(m_file->getSampleCount()) / m_file->getSampleRate() / m_file->getChannelCount());
    m_samples.resize(m_file->getSampleRate() * m_file->getChannelCount() / 2);
    return SoundStream::initialize(m_file->getChannelCount(), m_file->getSampleRate());
}
    
bool SoundFX::getDataChunk(cribys::SoundStream::Chunk &data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    data.m_samples = &m_samples[0];
    data.m_sampleCount = m_file->read(&m_samples[0], m_samples.size());
    return data.m_sampleCount == m_samples.size();
}
}
