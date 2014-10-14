//
//  Sound.cpp
//  HelloWorld
//
//  Created by Александр Коваль on 23.04.14.
//
//

#include "Sound.h"
#include "SoundBuffer.h"
#include "cocoa/CCZone.h"
#include "CCScheduler.h"
#include "platform/CCCommon.h"

namespace cribys {
Sound* Sound::create(SoundBuffer &buffer) {
    Sound* sound = new Sound(buffer);
    if (sound) {
        sound->autorelease();
        return sound;
    }
    CC_SAFE_DELETE(sound);
    return nullptr;
}

Sound* Sound::create(void) {
    Sound* sound = new Sound();
    if (sound) {
        sound->autorelease();
        return sound;
    }
    CC_SAFE_DELETE(sound);
    return nullptr;
}

Sound::Sound()
: AudioSource()
, m_loops(1)
, m_isDone(false)
, m_buffer(nullptr) {

}

Sound::Sound(SoundBuffer& buffer)
: AudioSource()
, m_loops(1)
, m_isDone(false)
, m_buffer(nullptr) {
    this->setBuffer(buffer);
}

Sound::~Sound() {
    this->stop();
    if (m_buffer) {
        m_buffer->detachSound(this);
        this->resetBuffer();
    }
}
    
void Sound::play(void) {
    using cocos2d::SEL_SCHEDULE;
    Status status = this->getStatus();
    if (STOPPED == status) {
        AudioTime time = AudioTime(int32_t(m_buffer->getFileSize()
                                           / (m_buffer->getSampleRate() * m_buffer->getChannelCount())));
        m_scheduler->scheduleSelector(schedule_selector(Sound::update), this, time.asSeconds(), false);
    } else if (PAUSED == status) {
        m_scheduler->resumeTarget(this);
    }
    AL_CHECK_ERRORS(alSourcePlay(m_source));
    m_isDone = !this->isPlaying();
}

void Sound::play(float startTime) {
    this->seek(startTime);
    this->play();
}
    
void Sound::play(float startTime, size_t loops) {
    if (INFINITY_LOOP == loops) {
        m_loops = loops;
        AL_CHECK_ERRORS(alSourcei(m_source, AL_LOOPING, true));
        this->play(startTime);
    } else if (loops > 0) {
        m_loops = loops;
        this->play(startTime);
    }
}

void Sound::pause(void) {
    AL_CHECK_ERRORS(alSourcePause(m_source));
    m_scheduler->pauseTarget(this);
    
    if (Status::STOPPED == this->getStatus()) {
        this->stop();
    }
}

void Sound::stop(void) {
    using cocos2d::SEL_SCHEDULE;
    m_isDone = true;
    if (INFINITY_LOOP == m_loops)
        AL_CHECK_ERRORS(alSourcei(m_source, AL_LOOPING, false));
    AL_CHECK_ERRORS(alSourceStop(m_source));
    m_scheduler->unscheduleSelector(schedule_selector(Sound::update), this);
}
    
void Sound::seek(float second) {
    Status status = this->getStatus();
    if (STOPPED != status)
        this->stop();
    AL_CHECK_ERRORS(alSourcef(m_source, AL_SEC_OFFSET, second));
    if (PLAYING == status)
        this->play();
}
    
const int16_t* Sound::getSamples(void) const {
    return m_buffer->getSamples();
}
    
unsigned int Sound::getSampleRate(void) const {
    return m_buffer->getSampleRate();
}
    
unsigned int Sound::getChannelCount(void) const {
    return m_buffer->getChannelCount();
}
    
unsigned long Sound::getFileSize(void) const {
    return m_buffer->getFileSize();
}
    
void Sound::update(float dt) {
    using cocos2d::SEL_SCHEDULE;
    this->processBuffers();
    if (m_isDone) {
        m_scheduler->unscheduleSelector(schedule_selector(Sound::update), this);
        if (m_selectorTarget && m_callback) {
            (m_selectorTarget->*m_callback)(this);
        }
    }
}
    
cocos2d::CCObject* Sound::copyWithZone(cocos2d::CCZone *zone) {
    cocos2d::CCZone* new_zone = nullptr;
    Sound* copy = nullptr;
    if (zone && zone->m_pCopyObject) {
        copy = static_cast<Sound*>(zone->m_pCopyObject);
    } else {
        copy = new Sound();
        zone = new_zone = new cocos2d::CCZone(copy);
    }
    SoundBuffer* buffer = const_cast<SoundBuffer*>(this->getBuffer());
    copy->setBuffer(*static_cast<SoundBuffer*>(buffer->copy()));
    copy->m_loops = m_loops;
    copy->setFileName(m_fileName);
    copy->setCallback(m_selectorTarget, m_callback);
    CC_SAFE_DELETE(new_zone);
    return copy;
}
    
void Sound::setBuffer(SoundBuffer &buffer) {
    if (m_buffer) {
        this->stop();
        this->resetBuffer();
        m_buffer->detachSound(this);
        m_buffer->release();
    }
    m_buffer = &buffer;
    m_buffer->retain();
    m_buffer->attachSound(this);
    AL_CHECK_ERRORS(alSourcei(m_source, AL_BUFFER, m_buffer->m_buffer));
}

const SoundBuffer* Sound::getBuffer(void) const {
    return m_buffer;
}
    
void Sound::resetBuffer(void) {
    this->stop();
    AL_CHECK_ERRORS(alSourcei(m_source, AL_BUFFER, 0));
    m_buffer->release();
    m_buffer = nullptr;
}
    
bool Sound::processBuffers(void) {
    if (!m_isDone) {
        ALint buffers_processed = 0;
        AL_CHECK_ERRORS(alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &buffers_processed));
        if (buffers_processed--) {
            if (m_loops <= 1) {
                m_isDone = true;
            } else {
                AL_CHECK_ERRORS(alSourcef(m_source, AL_SEC_OFFSET, 0.0f));
                AL_CHECK_ERRORS(alSourcePlay(m_source));
                if (INFINITY_LOOP != m_loops) m_loops--;
            }
        }
    }
    return m_isDone;
}
}



