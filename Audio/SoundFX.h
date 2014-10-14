//
//  SoundFX.h
//  CribysUtils
//
//  Created by Александр Коваль on 04.06.14.
//  Copyright (c) 2014 Cribys. All rights reserved.
//

#ifndef __CribysUtils__SoundFX__
#define __CribysUtils__SoundFX__

#include <vector>
#include "SoundStream.h"

namespace cribys {
class SoundFX : public SoundStream {
public:
    static SoundFX* create(void);
    
    static SoundFX* create(const char* pszFileName);
    
    static SoundFX* create(SoundFile* file);
    
    explicit SoundFX(void);
    
    virtual ~SoundFX(void);
    
    bool initWithFile(const char* pszFileName);
    
    bool initWithFile(SoundFile* file);
    
    virtual void stop(void);
    
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
#endif /* defined(__CribysUtils__SoundFX__) */
