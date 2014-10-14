#ifndef SOUNDCACHE_H
#define SOUNDCACHE_H

#include <set>
#include <thread>
#include <future>
#include <queue>
#include <functional>
#include "cocoa/CCObject.h"
#include "actions/CCActionInstant.h"

namespace cocos2d { class CCScheduler; }
namespace cribys {
class Sound;
class CCDictionary;
    
template <typename T>
class AsyncTask : public cocos2d::CCObject {
public:
    static AsyncTask* create(std::function<T(void)>);
    
    explicit AsyncTask();
    
    ~AsyncTask();
    
    virtual bool initWithFunction(std::function<T(void)>);
    
    virtual void execute(void);
    
    virtual void update(float dt);
    
    void setCompletionFunc(cocos2d::CCCallFuncO* callfunc);
    
    void setCompletionFunc(std::function<void(T)> callback);;
    
    T getResult(void);
        
    bool isRunning(void) const;
    
    bool isDone(void) const;
        
private:
    T m_result;
    bool m_isDone;
    bool m_isRunning;
    std::future<T> m_future;
    cocos2d::CCScheduler* m_scheduler;
    std::function<T(void)> m_function;
    cocos2d::CCCallFuncO* m_callfunc;
    std::function<void(T)> m_callback;
};

class SoundCache : public cocos2d::CCObject {
public:
    static SoundCache* sharedSoundCache(void);

    SoundCache(void);

    void addSound(const char* pszFileName, Sound* sound);
    
    void addSoundWithFile(const char* pszFileName);
    
    void addSoundWithFileAsync(const char* pszFileName);

    void addSoundWithFileAsync(const char* pszFileName,
                          cocos2d::CCCallFuncO* onComplete);
    
    void addSoundWithFileAsync(const char* pszFileName,
                          std::function<void(CCObject*)> onComplete);

    void addSoundsWithSet(cocos2d::CCSet* set);
    
    void addSoundsWithSetAsync(cocos2d::CCSet* set);

    void addSoundsWithSetAsync(cocos2d::CCSet* set, cocos2d::CCCallFuncO* onComplete);

    void addSoundsWithSetAsync(cocos2d::CCSet* set, std::function<void(CCObject*)> onComplete);

    void addSoundsWithSet(std::set<const char*>& set);
    
    void addSoundsWithSetAsync(std::set<const char*>& set);

    void addSoundsWithSetAsync(std::set<const char*>& set, cocos2d::CCCallFuncO* onComplete);

    void addSoundsWithSetAsync(std::set<const char*>& set, std::function<void(CCObject*)> onComplete);

    void removeSounds(void);

    void removeSoundByName(const char* pszFileName);

    Sound* getSoundByName(const char *pszName);

protected:
    virtual AsyncTask<Sound*>* initTaskWithFile(const char* pszFileName);
    
    virtual void start(void);
    
    virtual void stop(void);
    
    virtual void update(float dt);
    
private:
    cocos2d::CCDictionary* m_sounds;
    std::queue<AsyncTask<Sound*>*> m_tasks;
    cocos2d::CCScheduler* m_scheduler;
    std::future<Sound*> m_future;
    AsyncTask<Sound*>* m_task;
    bool m_isRunning;
};
}


#endif // SOUNDCACHE_H
