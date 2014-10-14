//
//  SoundStream.cpp
//  CribysUtils
//
//  Created by Александр Коваль on 22.05.14.
//  Copyright (c) 2014 Cribys. All rights reserved.
//

#include "Audio.h"
#include "SoundFile.h"
#include "SoundStream.h"
#include "CCScheduler.h"
//#include "ActionInstantAsync.h"

namespace cribys {
    
const int SoundStream::s_bufferCount = 3;
    
SoundStream::SoundStream(void)
: AudioSource()
, m_format(0)
, m_isDone(false)
, m_loops(1)
, m_thread(nullptr)
, m_sampleRate(0)
, m_channelCount(0)
, m_samplesProcessed(0) {
    using cocos2d::SEL_CallFunc;
    m_buffersProcessed = new bool[s_bufferCount];
    m_buffers = new unsigned int[s_bufferCount];
}
    
SoundStream::~SoundStream(void) {
    this->stop();
    CC_SAFE_DELETE_ARRAY(m_buffersProcessed);
    CC_SAFE_DELETE_ARRAY(m_buffers);
}
    
bool SoundStream::initialize(unsigned int channel_count, unsigned int sample_rate) {
    if (!channel_count || !sample_rate) return false;
    m_format = Audio::getFormatFromChannelCount(channel_count);
    if (m_format ==  0) {
        CCLOGERROR("unsupported number of channels %d", channel_count);
        return false;
    }
    m_sampleRate = sample_rate;
    m_channelCount = channel_count;
    return true;
}
                
void SoundStream::play(void) {
    using cocos2d::SEL_SCHEDULE;
    CCAssert(m_format, "sound parameters have not been inisialized");
    Status status = this->getStatus();
    if (!m_isStreaming && !m_thread) {
        m_isDone = false;
        m_isStreaming = true;
        AudioTime time = AudioTime(int32_t(this->getFileSize()
                                           / (this->getSampleRate() * this->getChannelCount())));
        m_scheduler->scheduleSelector(schedule_selector(SoundStream::update), this, time.asSeconds(), false);
        m_thread = new std::thread(&SoundStream::stream, this);
    } else if (PAUSED == status) {
        m_scheduler->resumeTarget(this);
        AL_CHECK_ERRORS(alSourcePlay(m_source));
    } else {
        AL_CHECK_ERRORS(alSourcePlay(m_source));
    }
}
    
void SoundStream::play(float startTime) {
    this->seek(startTime);
    m_samplesProcessed = static_cast<unsigned long>(startTime * m_sampleRate * m_channelCount);
    this->play();
}
    
void SoundStream::play(float startTime, size_t loops) {
    if (loops == INFINITY_LOOP) {
        m_loops = loops;
        this->play(startTime);
    } else {
        m_loops = loops;
        this->play(startTime);
    }
}
    
void SoundStream::pause(void) {
    AL_CHECK_ERRORS(alSourcePause(m_source));
    m_scheduler->pauseTarget(this);
}
    
void SoundStream::stop(void) {
    using cocos2d::SEL_SCHEDULE;
    m_isStreaming = false;
    m_samplesProcessed = 0;
    if (m_thread) {
        m_thread->join();
        CC_SAFE_DELETE(m_thread);
        AL_CHECK_ERRORS(alSourceStop(m_source));
        m_thread = nullptr;
    }
    m_scheduler->unscheduleSelector(schedule_selector(SoundStream::update), this);
}
        
void SoundStream::update(float dt) {
    using cocos2d::SEL_SCHEDULE;
    if (!m_isStreaming) {
        m_scheduler->unscheduleSelector(schedule_selector(SoundStream::update), this);
        if (m_selectorTarget && m_callback) {
            (m_selectorTarget->*m_callback)(this);
        }
        if (m_thread) {
            m_thread->join();
            CC_SAFE_DELETE(m_thread);
            m_thread = nullptr;
        }
    }
}
    
unsigned int SoundStream::getSampleRate(void) const {
    return m_sampleRate;
}
    
unsigned int SoundStream::getChannelCount(void) const {
    return m_channelCount;
}
    
void SoundStream::stream(void) {
    AL_CHECK_ERRORS(alGenBuffers(s_bufferCount, m_buffers));
    memset(m_buffersProcessed, 0, sizeof(*m_buffersProcessed));
    bool request_stop = this->queueBuffers();
    AL_CHECK_ERRORS(alSourcePlay(m_source));
    while (m_isStreaming) {
        Status status = AudioSource::getStatus();
        if (Status::STOPPED == status && !request_stop) {
            AL_CHECK_ERRORS(alSourcePlay(m_source));
        } else if (Status::STOPPED == status && request_stop) {
            AL_CHECK_ERRORS(alSourceStop(m_source));
            m_isStreaming = false;
        }
        if (!request_stop) request_stop = this->processBuffers();
        if (Status::STOPPED != AudioSource::getStatus()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    AL_CHECK_ERRORS(alSourceStop(m_source));
    this->seek(0.0f);
    this->unqueueBuffers();
    alSourcei(m_source, AL_BUFFER, 0);
    if (alIsBuffer(*m_buffers))
        alDeleteBuffers(s_bufferCount, m_buffers);
    memset(m_buffers, 0, sizeof(*m_buffers));
    m_isDone = true;
}
    
bool SoundStream::processBuffers(void) {
    bool request_stop = false;
    ALint buffers_processed = 0;
    AL_CHECK_ERRORS(alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &buffers_processed));
    while (buffers_processed--) {
        ALuint buffer;
        alSourceUnqueueBuffers(m_source, 1, &buffer);
        if (Audio::getError()) return true;
        unsigned int buffer_num = 0;
        while (m_buffers[buffer_num] != buffer)
            buffer_num++;

        if (m_buffersProcessed[buffer_num]) {
            m_samplesProcessed = 0;
            m_buffersProcessed[buffer_num] = false;
        } else {
            ALint size, bits;
            AL_CHECK_ERRORS(alGetBufferi(buffer, AL_SIZE, &size));
            AL_CHECK_ERRORS(alGetBufferi(buffer, AL_BITS, &bits));
            CCAssert(bits, "Bits in sound stream are 0: make sure that the audio format is not corrupt");
            m_samplesProcessed += size / (bits / 8);
        }
        if (m_isStreaming) {
            request_stop = this->queueBuffer(buffer_num);
        }
    }
    return request_stop;
}
    
bool SoundStream::queueBuffer(unsigned int buffer_num) {
    bool request_stop = false;
    Chunk data = { nullptr, 0 };
    if (!this->getDataChunk(data)) {
        m_buffersProcessed[buffer_num] = true;
        if (m_loops <= 1) {
            request_stop = true;
        } else {
            this->seek(0.0f);
            if (!data.m_samples || !data.m_sampleCount) {
                return queueBuffer(buffer_num);
            }
            if (INFINITY_LOOP != m_loops) m_loops--;
        }
    }
    if (data.m_samples && data.m_sampleCount) {
        unsigned int buffer = m_buffers[buffer_num];
        ALsizei size = static_cast<ALsizei>(data.m_sampleCount) * sizeof(int16_t);
        AL_CHECK_ERRORS(alBufferData(buffer, m_format, data.m_samples, size, m_sampleRate));
        AL_CHECK_ERRORS(alSourceQueueBuffers(m_source, 1, &buffer));
    }
    return request_stop;
}
    
bool SoundStream::queueBuffers(void) {
    bool request_stop = false;
    for (int index = 0; index < s_bufferCount; index++) {
        request_stop = this->queueBuffer(index);
    }
    return request_stop;
}
    
void SoundStream::unqueueBuffers(void) {
    ALint queued;
    AL_CHECK_ERRORS(alGetSourcei(m_source, AL_BUFFERS_QUEUED, &queued));
//    AL_CHECK_ERRORS(alSourceUnqueueBuffers(m_source, queued, m_buffers));
    alSourceUnqueueBuffers(m_source, queued, m_buffers);
}
}
