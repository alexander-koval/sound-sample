//
//  Music.cpp
//  CribysUtils
//
//  Created by Александр Коваль on 26.05.14.
//  Copyright (c) 2014 Cribys. All rights reserved.
//

#include "Music.h"
#include "SoundFile.h"
#include "cocoa/CCZone.h"

namespace cribys {

Music* Music::create(void) {
    Music* music = new Music();
    music->autorelease();
    return music;
}
    
Music* Music::create(const char *pszFileName) {
    Music* music = new Music();
    if (music->initWithFile(pszFileName)) {
        music->autorelease();
        return music;
    }
    CC_SAFE_DELETE(music);
    return nullptr;
}
    
Music* Music::create(cribys::SoundFile *file) {
    Music* music = new Music();
    if (music->initWithFile(file)) {
        music->autorelease();
        return music;
    }
    CC_SAFE_DELETE(music);
    return nullptr;
}
    
Music::Music(void)
: SoundStream()
, m_file(nullptr) {
        
}
    
Music::~Music(void) {
    CCLOG("~MUSIC: %s", this->getFileName());
    CC_SAFE_DELETE_ARRAY(m_fileName);
    CC_SAFE_RELEASE(m_file);
}
    
bool Music::initWithFile(const char *pszFileName) {
    SoundFile* file = SoundFile::create(pszFileName);
    return this->initWithFile(file);
}
    
bool Music::initWithFile(cribys::SoundFile *file) {
    m_file = file;
    if (!m_file) return false;
    unsigned int channel_count = file->getChannelCount();
    unsigned int sample_rate   = file->getSampleRate();
    char* name = new char[strlen(file->getName())];
    strcpy(name, file->getName());
    m_fileName = name;
    CCLOG("MUSIC: %s, sample_count %ld, channel_count %u, sample_rate %u", file->getName(), file->getSampleCount(), file->getChannelCount(), file->getSampleRate());
    m_file->retain();
    return this->initialize(channel_count, sample_rate);
}
    
void Music::stop(void) {
    SoundStream::stop();
    this->seek(0.0f);
}
    
void Music::seek(float second) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_file->seek(second);
}
    
cocos2d::CCObject* Music::copyWithZone(cocos2d::CCZone *zone) {
    cocos2d::CCZone* new_zone = nullptr;
    Music* copy = nullptr;
    if (zone && zone->m_pCopyObject) {
        copy = static_cast<Music*>(zone->m_pCopyObject);
    } else {
        char* filename = new char[strlen(this->getFileName())];
        strcpy(filename, this->getFileName());
        copy = Music::create(filename);
        zone = new_zone = new cocos2d::CCZone(copy);
    }
    copy->m_samples = m_samples;
    copy->initialize(this->getChannelCount(), this->getSampleRate());
    CC_SAFE_DELETE(new_zone);
    return copy;
}
    
const int16_t* Music::getSamples(void) const {
    return m_samples.empty() ? nullptr : &m_samples[0];
}
    
std::size_t Music::getSampleCount(void) const {
    return m_samples.size();
}
    
unsigned long Music::getFileSize(void) const {
    return m_file->getMemorySize();
}
        
bool Music::initialize(unsigned int channel_count, unsigned int sample_rate) {
    m_duration = AudioTime(static_cast<float>(m_file->getSampleCount()) / m_file->getSampleRate() / m_file->getChannelCount());
    m_samples.resize(m_file->getSampleRate() * m_file->getChannelCount());
    return SoundStream::initialize(m_file->getChannelCount(), m_file->getSampleRate());
}
    
bool Music::getDataChunk(cribys::SoundStream::Chunk &data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    data.m_samples = &m_samples[0];
    data.m_sampleCount = m_file->read(&m_samples[0], m_samples.size());
    return data.m_sampleCount == m_samples.size();
}
}