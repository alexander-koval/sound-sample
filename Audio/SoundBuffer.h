//
//  SoundBuffer.h
//  HelloWorld
//
//  Created by Александр Коваль on 25.04.14.
//
//

#ifndef __HelloWorld__SoundBuffer__
#define __HelloWorld__SoundBuffer__

#include <set>
#include <vector>
#include "Audio.h"
#include "cocoa/CCObject.h"

namespace cribys {
class Sound;
class SoundFile;
class SoundBuffer : public cocos2d::CCObject {
public:
    friend class Sound;
    
    static SoundBuffer* create(void);
    
    static SoundBuffer* create(const char* pszFileName);
    
    static SoundBuffer* create(SoundFile* file);

    SoundBuffer();
    
    ~SoundBuffer();
    
    virtual cocos2d::CCObject* copyWithZone(cocos2d::CCZone* zone);

    bool initWithFile(const char* pszFileName);
    
    bool initWithFile(SoundFile* file);
    
    const int16_t* getSamples(void) const;
    
    std::size_t getSampleCount(void) const;
    
    unsigned int getSampleRate(void) const;
    
    unsigned int getChannelCount(void) const;
    
    AudioTime getDuration(void) const;
    
    AudioTime getElapsedTime(void) const;

    unsigned long getFileSize(void) const;

    void attachSound(Sound* sound);
    
    void detachSound(Sound* sound);
    
private:
    bool initialize(unsigned int channel_count, unsigned int sample_rate);
    
    AudioTime m_duration;
    unsigned int m_buffer;
    unsigned long m_memorySize;
    std::vector<int16_t> m_samples;
    std::set<Sound*> m_sounds;
};
};

#endif /* defined(__HelloWorld__SoundBuffer__) */
