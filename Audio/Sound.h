//
//  Sound.h
//  HelloWorld
//
//  Created by Александр Коваль on 23.04.14.
//
//

#ifndef __HelloWorld__Sound__
#define __HelloWorld__Sound__

#include <thread>
#include <limits>
#include "Audio.h"
#include "actions/CCActionInstant.h"

namespace cribys {
class SoundBuffer;
class SoundManager;
class Sound : public AudioSource {
public:
    friend class SoundBuffer;
    
    static Sound* create(SoundBuffer& buffer);
    
    static Sound* create(void);
    
    explicit Sound(SoundBuffer& buffer);

    explicit Sound(void);
    
    virtual ~Sound(void);
    
    virtual Status getStatus(void) const {
        return AudioSource::getStatus();
    };
    
    virtual void update(float dt);
    
    virtual CCObject* copyWithZone(cocos2d::CCZone* zone);
    
    /**
     * Play back the sound
     */
    virtual void play(void);
    
    /**
     *  Play back the sound
     *  param: startTime - The initial position in seconds at which playback should start.
     */
    virtual void play(float startTime);

    /**
     *  Play back the sound
     *  param: startTime - The initial position in seconds at which playback should start.
     *  param: loops     - Defines the number of times a sound loops back to the startTime value before the sound stop playback (set parameter INFINITY_LOOP for endless playing)
     */
    virtual void play(float startTime, size_t loops);
    
    virtual void pause(void);
    
    virtual void stop(void);
    
    virtual void seek(float second);
    
    virtual const int16_t* getSamples(void) const;
    
    virtual unsigned int getSampleRate(void) const;
    
    virtual unsigned int getChannelCount(void) const;
    
    virtual unsigned long getFileSize(void) const;
    
    void setBuffer(SoundBuffer& buffer);
    
    const SoundBuffer* getBuffer(void) const;
    
//    void setCallback(cocos2d::CCCallFuncO* callback);
    
    void resetBuffer(void);
    
private:
    bool processBuffers(void);
    
    size_t m_loops;
    bool m_isDone;
//    std::thread* m_thread;
    SoundBuffer* m_buffer;
//    const char* m_fileName;
//    cocos2d::CCCallFuncO* m_callback;
};
};

#endif /* defined(__HelloWorld__Sound__) */
