//
//  SoundStream.h
//  CribysUtils
//
//  Created by Александр Коваль on 22.05.14.
//  Copyright (c) 2014 Cribys. All rights reserved.
//

#ifndef __CribysUtils__SoundStream__
#define __CribysUtils__SoundStream__

#include <mutex>
#include <thread>
#include <vector>
#include "Audio.h"

namespace cribys {
class SoundFile;
class SoundStream : public AudioSource {
public:
    explicit SoundStream(void);
    
    virtual ~SoundStream(void);
    
    virtual void play(void);
    
    virtual void play(float startTime);
    
    virtual void play(float startTime, size_t loops);
    
    virtual void pause(void);
    
    virtual void stop(void);
        
    virtual void update(float dt);
    
    virtual unsigned int getSampleRate(void) const;
    
    virtual unsigned int getChannelCount(void) const;
    
protected:
    struct Chunk {
        const int16_t* m_samples;
        std::size_t m_sampleCount;
    };
    
    virtual bool initialize(unsigned int channel_count, unsigned int sample_rate);
    
    virtual bool getDataChunk(Chunk& data) = 0;

    int m_format;
    bool m_isDone;
    size_t m_loops;
    
private:
    void stream(void);
    
    bool processBuffers(void);
    
    bool queueBuffer(unsigned int buffer_num);
    
    bool queueBuffers(void);
    
    void unqueueBuffers(void);
    
    std::thread* m_thread;
    unsigned int m_sampleRate;
    unsigned int m_channelCount;
    static const int s_bufferCount;
    unsigned long m_samplesProcessed;
    bool* m_buffersProcessed;
    unsigned int* m_buffers;
};
}
#endif /* defined(__CribysUtils__SoundStream__) */
