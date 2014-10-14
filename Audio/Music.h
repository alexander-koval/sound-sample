//
//  Music.h
//  CribysUtils
//
//  Created by Александр Коваль on 26.05.14.
//  Copyright (c) 2014 Cribys. All rights reserved.
//

#ifndef __CribysUtils__Music__
#define __CribysUtils__Music__

#include <vector>
#include <mutex>
#include "SoundStream.h"

namespace cribys {
class SoundFile;
class Music : public SoundStream {
public:
    static Music* create(void);
    
    static Music* create(const char* pszFileName);
    
    static Music* create(SoundFile* file);
    
    explicit Music(void);
    
    virtual ~Music(void);
    
    virtual void stop(void);
    
    bool initWithFile(const char* pszFileName);
    
    bool initWithFile(SoundFile* file);
    
    virtual void seek(float second);
    
    virtual CCObject* copyWithZone(cocos2d::CCZone* zone);
    
    virtual const int16_t* getSamples(void) const;

    virtual unsigned long getFileSize(void) const;
    
    std::size_t getSampleCount(void) const;

protected:
    virtual bool initialize(unsigned int channel_count, unsigned int sample_rate);
    
    virtual bool getDataChunk(Chunk& data);
    
private:
    SoundFile* m_file;
    std::mutex m_mutex;
    AudioTime m_duration;
    std::vector<int16_t> m_samples;
};
}

#endif /* defined(__CribysUtils__Music__) */
