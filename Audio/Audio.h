//
//  Audio.h
//  HelloWorld
//
//  Created by Александр Коваль on 23.04.14.
//
//

#ifndef __HelloWorld__Audio__
#define __HelloWorld__Audio__

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include "cocoa/CCObject.h"
#include "ccMacros.h"

#if COCOS2D_DEBUG > 0
#define AL_CHECK_ERRORS(x) {                                        \
    CC_UNUSED_PARAM(x);                                             \
    const char* error = Audio::getError();                          \
    CCAssert(!error, error);             \
};
#else
#define AL_CHECK_ERRORS(x) { CC_UNUSED_PARAM(x); };
#endif

#define INFINITY_LOOP std::numeric_limits<int>::max()

namespace cocos2d { class CCActionManager; }
namespace cocos2d { class CCScheduler; }
namespace cocos2d { class CCAction; }
namespace cribys {
class AudioTime {
public:
    AudioTime(void);
        
    explicit AudioTime(int64_t microseconds);
        
    explicit AudioTime(int32_t milliseconds);
        
    explicit AudioTime(float seconds);
        
    float asSeconds(void) const;
        
    int32_t asMilliseconds(void) const;
        
    int64_t asMicroseconds(void) const;
        
private:
    int64_t m_microseconds;
};

class AudioSource : public cocos2d::CCObject {
public:
    enum Status {
        PLAYING = 0,
        STOPPED = 1,
        PAUSED  = 2
    };
    
    AudioSource(void);
    
    virtual ~AudioSource(void);
    
    virtual void play(void) = 0;
    
    virtual void play(float startTime) = 0;
    
    virtual void play(float startTime, size_t loops) = 0;
    
    virtual void pause(void) = 0;
    
    virtual void stop(void) = 0;
    
    virtual void seek(float second) = 0;
    
    virtual const int16_t* getSamples(void) const = 0;
    
    virtual unsigned int getSampleRate(void) const = 0;
    
    virtual unsigned int getChannelCount(void) const = 0;
    
    virtual unsigned long getFileSize(void) const = 0;
    
    virtual Status getStatus(void) const;
    
    void setCallback(cocos2d::CCObject* pSelectorTarget, cocos2d::SEL_CallFuncO selector);
    
    void setFileName(const char* name);
    
    const char* getFileName(void) const;
        
    bool isPlaying(void) const;
    
    bool isStreaming(void) const;
    
    AudioTime getElapsedTime(void) const;

    void setRelative(bool relative);
    
    bool isRelative(void) const;
    
    void setVolume(float volume);
    
    float getVolume(void) const;
    
    void setSpeed(float speed);
    
    float getSpeed(void) const;
    
    void setFade(float fade);
    
    float getFade(void);
    
protected:
    cocos2d::CCObject* m_selectorTarget;
    
    cocos2d::SEL_CallFuncO m_callback;
    
    cocos2d::CCActionManager* m_actionManager;
    
    cocos2d::CCScheduler* m_scheduler;
    
    unsigned int m_source;
    
    const char* m_fileName;
    
    bool m_isStreaming;
    
private:
    virtual void setActionManager(cocos2d::CCActionManager* action_manager);
    
    virtual cocos2d::CCActionManager* getActionManager(void);
    
    cocos2d::CCAction* runAction(cocos2d::CCAction* action);
    
    void stopAllActions(void);
    
    void stopAction(cocos2d::CCAction* action);
    
    void stopActionByTag(int tag);
    
    cocos2d::CCAction* getActionByTag(int tag);
    
    unsigned int numberOfRunningActions(void);
};

class Audio {
public:
    static const char* getError(void);
    
    static int getFormatFromChannelCount(unsigned int channel_count);
private:
    Audio(void) = delete;
};

class AudioDevice : public cocos2d::CCObject {
public:
    AudioDevice();
    
    ~AudioDevice();
    
private:
    ALCdevice* m_device;
    ALCcontext* m_context;
};
};

#endif /* defined(__HelloWorld__Audio__) */
