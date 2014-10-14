#include <cstring>
#include <algorithm>
#include "SoundCache.h"
#include "CCDirector.h"
#include "CCScheduler.h"
#include "cocoa/CCDictionary.h"
#include "cocoa/CCString.h"
#include "Sound.h"
#include "SoundFile.h"
#include "SoundBuffer.h"

namespace cribys {
using namespace cocos2d;
static SoundCache* s_soundCache = nullptr;
SoundCache* SoundCache::sharedSoundCache(void) {
    if (!s_soundCache) {
        s_soundCache = new SoundCache();
    }
    return s_soundCache;
}

SoundCache::SoundCache(void)
: cocos2d::CCObject()
, m_sounds(nullptr)
, m_task(nullptr)
, m_scheduler(nullptr)
, m_isRunning(false) {
    cocos2d::CCDirector* director = cocos2d::CCDirector::sharedDirector();
    m_scheduler = director->getScheduler();
    m_sounds = cocos2d::CCDictionary::create();
    m_sounds->retain();
}

void SoundCache::addSound(const char* pszFileName, Sound *sound) {
    if (m_sounds->objectForKey(pszFileName)) {
        m_sounds->setObject(sound, pszFileName);        
    }
}
    
void SoundCache::addSoundWithFile(const char *pszFileName) {
    SoundBuffer* buffer = SoundBuffer::create(pszFileName);
    if (buffer) {
        Sound* sound = Sound::create(*buffer);
        sound->setFileName(pszFileName);
        m_sounds->setObject(sound, pszFileName);
    }
}
    
void SoundCache::addSoundWithFileAsync(const char *pszFileName) {
    if (m_sounds->objectForKey(pszFileName)) return;
    AsyncTask<Sound*>* task = this->initTaskWithFile(pszFileName);
    task->setCompletionFunc([this](Sound* sound) -> void {
        m_sounds->setObject(sound, sound->getFileName());
        sound->release();
    });
    m_tasks.push(task);
    this->start();
}

void SoundCache::addSoundWithFileAsync(const char *pszFileName,
                                  cocos2d::CCCallFuncO* onComplete) {
    Sound* sound = static_cast<Sound*>(m_sounds->objectForKey(pszFileName));
    if (!sound) {
        onComplete->retain();
        AsyncTask<Sound*>* task = this->initTaskWithFile(pszFileName);
        task->setCompletionFunc([this, onComplete](Sound* sound) -> void {
            m_sounds->setObject(sound, sound->getFileName());
            onComplete->setObject(sound);
            onComplete->execute();
            onComplete->release();
            sound->release();
        });
        m_tasks.push(task);
        this->start();
    } else {
        onComplete->setObject(sound);
        onComplete->execute();
    }
}
    
void SoundCache::addSoundWithFileAsync(const char *pszFileName,
                                  std::function<void (CCObject *)> onComplete) {
    Sound* sound = static_cast<Sound*>(m_sounds->objectForKey(pszFileName));
    if (!sound) {
        AsyncTask<Sound*>* task = this->initTaskWithFile(pszFileName);
        task->setCompletionFunc([this, onComplete](Sound* sound) -> void {
            m_sounds->setObject(sound, sound->getFileName());
            onComplete(sound);
            sound->release();
        });
        m_tasks.push(task);
        this->start();
    } else {
        onComplete(sound);
    }
}
    
void SoundCache::addSoundsWithSet(cocos2d::CCSet *set) {
    std::for_each(set->begin(), set->end(), [this](cocos2d::CCObject* object) {
        cocos2d::CCString* str = static_cast<cocos2d::CCString*>(object);
        this->addSoundWithFile(str->getCString());
    });
}

void SoundCache::addSoundsWithSetAsync(cocos2d::CCSet *set) {
    std::for_each(set->begin(), set->end(), [this](cocos2d::CCObject* object) {
        cocos2d::CCString* str = static_cast<cocos2d::CCString*>(object);
        this->addSoundWithFileAsync(str->getCString());
    });
}
    
void SoundCache::addSoundsWithSetAsync(cocos2d::CCSet *set, cocos2d::CCCallFuncO *onComplete) {
    cocos2d::CCDictionary* dictionary = cocos2d::CCDictionary::create();
    dictionary->retain();
    int* counter = new int(set->count());
    onComplete->retain();
    for (CCSetIterator it = set->begin(); it != set->end(); it++) {
        cocos2d::CCString* str = static_cast<cocos2d::CCString*>(*it);
        AsyncTask<Sound*>* task = this->initTaskWithFile(str->getCString());
        task->setCompletionFunc([this, counter, dictionary, onComplete](Sound* sound) -> void {
            m_sounds->setObject(sound, sound->getFileName());
            dictionary->setObject(sound, sound->getFileName());
            if (--(*counter) <= 0) {
                onComplete->setObject(dictionary);
                onComplete->execute();
                dictionary->release();
                onComplete->release();
                delete counter;
            }
            sound->release();
        });
        m_tasks.push(task);
    }
    this->start();
}
    
void SoundCache::addSoundsWithSetAsync(cocos2d::CCSet *set, std::function<void (CCObject *)> onComplete) {
    cocos2d::CCDictionary* dictionary = cocos2d::CCDictionary::create();
    dictionary->retain();
    int* counter = new int(set->count());
    for (CCSetIterator it = set->begin(); it != set->end(); it++) {
        cocos2d::CCString* str = static_cast<cocos2d::CCString*>(*it);
        AsyncTask<Sound*>* task = this->initTaskWithFile(str->getCString());
        task->setCompletionFunc([this, counter, dictionary, onComplete](Sound* sound) -> void {
            m_sounds->setObject(sound, sound->getFileName());
            dictionary->setObject(sound, sound->getFileName());
            if (--(*counter) <= 0) {
                onComplete(dictionary);
                delete counter;
            }
            sound->release();
        });
        m_tasks.push(task);
    }
    this->start();
}
    
void SoundCache::addSoundsWithSet(std::set<const char *> &set) {
    std::for_each(set.begin(), set.end(), [this](const char* name) {
        this->addSoundWithFile(name);
    });
}
    
void SoundCache::addSoundsWithSetAsync(std::set<const char *> &set) {
    std::for_each(set.begin(), set.end(), [this](const char* name) {
        this->addSoundWithFileAsync(name);
    });
}
    
void SoundCache::addSoundsWithSetAsync(std::set<const char *> &set, cocos2d::CCCallFuncO *onComplete) {
    cocos2d::CCDictionary* dictionary = cocos2d::CCDictionary::create();
    dictionary->retain();
    int* counter = new int(set.size());
    for (std::set<const char*>::iterator it = set.begin(); it != set.end(); it++) {
        AsyncTask<Sound*>* task = this->initTaskWithFile(*it);
        task->setCompletionFunc([this, counter, dictionary, onComplete](Sound* sound) -> void {
            m_sounds->setObject(sound, sound->getFileName());
            dictionary->setObject(sound, sound->getFileName());
            if (--(*counter) <= 0) {
                onComplete->setObject(dictionary);
                onComplete->execute();
                dictionary->release();
                onComplete->release();
                delete counter;
            }
            sound->release();
        });
        m_tasks.push(task);
    }
    onComplete->retain();
    this->start();
}
    
void SoundCache::addSoundsWithSetAsync(std::set<const char *> &set, std::function<void (CCObject *)> onComplete) {
    cocos2d::CCDictionary* dictionary = cocos2d::CCDictionary::create();
    dictionary->retain();
    int* counter = new int(set.size());
    for (std::set<const char*>::iterator it = set.begin(); it != set.end(); it++) {
        AsyncTask<Sound*>* task = this->initTaskWithFile(*it);
        task->setCompletionFunc([this, counter, dictionary, onComplete](Sound* sound) -> void {
            m_sounds->setObject(sound, sound->getFileName());
            dictionary->setObject(sound, sound->getFileName());
            if (--(*counter) <= 0) {
                onComplete(dictionary);
                delete counter;
            }
            sound->release();
        });
        m_tasks.push(task);
    }
    this->start();
}
    
void SoundCache::start(void) {
    if (!m_isRunning) {
        m_isRunning = true;
        m_scheduler->scheduleUpdateForTarget(this, 0, false);
    }
}
    
void SoundCache::stop(void) {
    m_isRunning = false;
    m_scheduler->unscheduleAllForTarget(this);
}
    
void SoundCache::update(float dt) {
    AsyncTask<Sound*>* current_task = m_tasks.front();
    if (current_task && m_tasks.size() > 0) {
        if (!current_task->isRunning() && current_task->isDone()) {
            m_tasks.pop();
            CC_SAFE_DELETE(current_task);
        } else if (!current_task->isRunning() && !current_task->isDone()) {
            current_task->execute();
        }
    } else {
        this->stop();
    }
}

void SoundCache::removeSounds(void) {
    m_sounds->removeAllObjects();
}

void SoundCache::removeSoundByName(const char *pszFileName) {
    m_sounds->removeObjectForKey(pszFileName);
}
    
Sound* SoundCache::getSoundByName(const char *pszName) {
    Sound* sound = static_cast<Sound*>(m_sounds->objectForKey(pszName));
    if (!sound) CCLOGERROR("SoundCache: Sound '%s' not found", pszName);
    return sound;
}
    
AsyncTask<Sound*>* SoundCache::initTaskWithFile(const char* pszFileName) {
    size_t len = strlen(pszFileName) + 1;
    char* file_name = new char[len];
    std::strcpy(file_name, pszFileName);
    file_name[len - 1] = '\0';
    AsyncTask<Sound*>* task = AsyncTask<Sound*>::create([file_name]() -> Sound* {
        SoundFile* sound_file = SoundFile::create(file_name);
        if (sound_file) {
            SoundBuffer* buffer = SoundBuffer::create(sound_file);
            Sound* sound = Sound::create(*buffer);
            sound->setFileName(file_name);
            sound->retain();
            return sound;
        }
        return nullptr;
    });
    task->retain();
    return task;
}


template <typename T>
AsyncTask<T>* AsyncTask<T>::create(std::function<T ()> func) {
    AsyncTask<T>* task = new AsyncTask<T>();
    if (task && task->initWithFunction(func)) {
        task->autorelease();
        return task;
    }
    CC_SAFE_DELETE(task);
    return nullptr;
}
    
template <typename T>
AsyncTask<T>::AsyncTask()
: cocos2d::CCObject()
, m_result(nullptr)
, m_isDone(false)
, m_isRunning(false)
, m_scheduler(nullptr)
, m_function(nullptr)
, m_callfunc(nullptr)
, m_callback(nullptr) {
    cocos2d::CCDirector* director = cocos2d::CCDirector::sharedDirector();
    m_scheduler = director->getScheduler();
}
    
template <typename T>
AsyncTask<T>::~AsyncTask() {
    m_result = nullptr;
    m_scheduler = nullptr;
    m_function = nullptr;
    m_callback = nullptr;
    CC_SAFE_RELEASE(m_callfunc);
}
    
template <typename T>
bool AsyncTask<T>::initWithFunction(std::function<T ()> func) {
    m_function = func;
    return true;
}
    
template <typename T>
void AsyncTask<T>::execute() {
    if (!m_isRunning) {
        std::packaged_task<T(void)> task(m_function);
        m_future = task.get_future();
        std::thread thread(std::move(task));
        thread.detach();
        m_scheduler->scheduleUpdateForTarget(this, 0, m_isRunning);
        m_isRunning = true;
    }
}
    
template <typename T>
void AsyncTask<T>::update(float dt) {
    if (m_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        m_scheduler->unscheduleAllForTarget(this);
        m_result = m_future.get();
        if (m_callback) {
            m_callback(std::move(m_result));
        } else if (m_callfunc) {
            m_callfunc->setObject(std::move(m_result));
            m_callfunc->execute();
        }
        m_isRunning = false;
        m_isDone = true;
    }
}
    
template <typename T>
void AsyncTask<T>::setCompletionFunc(cocos2d::CCCallFuncO *callfunc) {
    m_callfunc = callfunc;
    m_callfunc->retain();
}
    
template <typename T>
void AsyncTask<T>::setCompletionFunc(std::function<void (T)> func) {
    m_callback = func;
}
    
template <typename T>
T AsyncTask<T>::getResult() {
    return m_result;
}
    
template <typename T>
bool AsyncTask<T>::isRunning() const {
    return m_isRunning;
}
    
template <typename T>
bool AsyncTask<T>::isDone() const {
    return m_isDone;
}
}
