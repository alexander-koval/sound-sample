//
//  SoundManager.cpp
//  SoundEngine
//
//  Created by Александр Коваль on 09.05.14.
//  Copyright (c) 2014 Александр Коваль. All rights reserved.
//

#include "SoundManager.h"
#include "cocoa/CCDictionary.h"
#include "SoundCache.h"
#include "Sound.h"
#include "Music.h"
#include "SoundFX.h"

namespace cribys {
    
const SoundManager::PlayMode SoundManager::PlayMode::SINGLE = SoundManager::PlayMode();
SoundManager* SoundManager::s_sharedSoundManager = nullptr;

SoundPool* SoundPool::create() {
    SoundPool* pool = new SoundPool();
    pool->autorelease();
    return pool;
}

SoundPool::SoundPool()
: cocos2d::CCObject() {

}
    
SoundPool::~SoundPool() {
    for (std::vector<AudioSource*>::iterator it = m_pool.begin(); it != m_pool.end(); it++) {
        (*it)->stop();
        (*it)->release();
    }
    m_pool.clear();
}
    
AudioSource* SoundPool::getSound(void) {
    AudioSource* sound = nullptr;
    for (std::vector<AudioSource*>::iterator it = m_pool.begin(); it != m_pool.end(); it++) {
        if (!(*it)->isPlaying()) sound = *it;
    }
    if (!sound) {
        std::vector<AudioSource*>::iterator it = m_pool.begin();
        std::advance(it, rand() % m_pool.size());
        sound = *it;
    }
    return sound;
}
    
void SoundPool::putSound(cribys::AudioSource *sound) {
    sound->retain();
    m_pool.push_back(sound);
}
    
bool SoundPool::isPlaying(void) {
    for (std::vector<AudioSource*>::iterator it = m_pool.begin(); it != m_pool.end(); it++) {
        if ((*it)->isPlaying())
            return true;
    }
    return false;
}
    
bool SoundPool::isPaused(void) {
    for (std::vector<AudioSource*>::iterator it = m_pool.begin(); it != m_pool.end(); it++) {
        if ((*it)->getStatus() == AudioSource::PAUSED)
            return true;
    }
    return false;
}
    
std::vector<AudioSource*> SoundPool::getPlaying(void) {
    std::vector<AudioSource*> sounds;
    for (std::vector<AudioSource*>::iterator it = m_pool.begin(); it != m_pool.end(); it++) {
        if ((*it)->isPlaying()) sounds.push_back(*it);
    }
    return sounds;
}
    
std::vector<AudioSource*> SoundPool::getPaused(void) {
    std::vector<AudioSource*> sounds;
    for (std::vector<AudioSource*>::iterator it = m_pool.begin(); it != m_pool.end(); it++) {
        if ((*it)->getStatus() == AudioSource::PAUSED) sounds.push_back(*it);
    }
    return sounds;
}
    
void SoundPool::stopAll(void) {
    for (std::vector<AudioSource*>::iterator it = m_pool.begin(); it != m_pool.end(); it++) {
        if ((*it)->isPlaying()) (*it)->stop();
    }
}
    
void SoundPool::pauseAll(void) {
    for (std::vector<AudioSource*>::iterator it = m_pool.begin(); it != m_pool.end(); it++) {
        if ((*it)->isPlaying()) (*it)->pause();
    }
}
    
size_t SoundPool::getSize(void) {
    return m_pool.size();
}
    
void SoundPool::setSize(size_t size) {
    m_pool.resize(size);
}
    
SoundManager::PlayMode::PlayMode(size_t count /*= 1*/) {
    m_count = count;
}
    
const SoundManager::PlayMode SoundManager::PlayMode::MULTIPLE(size_t count) {
    return PlayMode(count);
}
    
size_t SoundManager::PlayMode::getCount(void) const {
    return m_count;
}
    
SoundManager* SoundManager::sharedSoundManager(void) {
    if (!s_sharedSoundManager) {
        s_sharedSoundManager = new SoundManager();
    }
    return s_sharedSoundManager;
}
    
void SoundManager::end(void) {
    if (s_sharedSoundManager) {
        CC_SAFE_DELETE(s_sharedSoundManager);
        s_sharedSoundManager = nullptr;
    }
}
    
SoundManager::SoundManager(void)
: cocos2d::CCObject(), m_isEnabled(true), m_musicVolume(1.0f), m_effectsVolume(1.0f) {
    using cocos2d::CCDictionary;
    m_sounds = CCDictionary::create();
    m_sounds->retain();
    m_music = CCDictionary::create();
    m_music->retain();
    m_effects = CCDictionary::create();
    m_effects->retain();
}
    
SoundManager::~SoundManager(void) {
    this->stopAll();
    CC_SAFE_RELEASE(m_sounds);
    CC_SAFE_RELEASE(m_music);
    CC_SAFE_RELEASE(m_effects);
    for (std::map<size_t, InfoMap*>::iterator it = m_packages.begin(); it != m_packages.end(); it++) {
        for (std::map<size_t, SoundInfo*>::iterator info_it = it->second->begin(); info_it != it->second->end(); info_it++) {
            CC_SAFE_DELETE(info_it->second);
        }
        it->second->clear();
        CC_SAFE_DELETE(it->second);
    }
    m_packages.clear();
}
    
bool SoundManager::addSoundInfo(size_t sound_id,
                                size_t package_id,
                                const char *file_name,
                                const SoundManager::SoundType type,
                                const SoundManager::PlayMode mode) {
    using cocos2d::CCArray;
    using cocos2d::SEL_CallFuncO;
    AudioSource* audio = nullptr;
    if (SoundType::EFFECT == type) {
        audio = SoundCache::sharedSoundCache()->getSoundByName(file_name);
    } else if (SoundType::MUSIC == type) {
        audio = Music::create(file_name);
    } else if (SoundType::STREAM_SFX == type) {
        audio = SoundFX::create(file_name);
    }
    if (!audio) return false;
    SoundInfo* sound_info = new SoundInfo(package_id, sound_id, file_name, type, mode);
    InfoMap* info_map = m_packages[package_id];
    if (!info_map) {
        info_map = new InfoMap();
        m_packages[package_id] = info_map;
    }
    (*info_map)[sound_id] = sound_info;
    SoundPool* sound_pool = SoundPool::create();
    sound_pool->putSound(audio);
    for (size_t index = 0; index < mode.getCount() - 1; index++) {
        AudioSource* copy = static_cast<AudioSource*>(audio->copy());
        sound_pool->putSound(copy);
    }
    m_sounds->setObject(sound_pool, file_name);
    if (SoundType::EFFECT == type || SoundType::STREAM_SFX == type) {
        m_effects->setObject(CCArray::create(), file_name);
    } else if (SoundType::MUSIC == type) {
        m_music->setObject(CCArray::create(), file_name);
    }
    return true;
}
    
void SoundManager::removeAll(void) {
    this->removeMusic();
    this->removeEffects();
}
    
void SoundManager::removeEffects(void) {
    for (std::map<size_t, InfoMap*>::iterator package_iterator = m_packages.begin(); package_iterator != m_packages.end(); package_iterator++) {
        for (std::map<size_t, SoundInfo*>::iterator it = package_iterator->second->begin(); it != package_iterator->second->end(); it++) {
            SoundInfo* info = it->second;
            package_iterator->second->erase(it);
            if (SoundType::EFFECT == info->type || SoundType::STREAM_SFX == info->type) {
                m_effects->removeObjectForKey(info->fileName);
                m_sounds->removeObjectForKey(info->fileName);
            }
            CC_SAFE_DELETE(info);
            it->second = nullptr;
        }
    }
}
    
void SoundManager::removeMusic(void) {
    for (std::map<size_t, InfoMap*>::iterator package_iterator = m_packages.begin(); package_iterator != m_packages.end(); package_iterator++) {
        for (std::map<size_t, SoundInfo*>::iterator it = package_iterator->second->begin(); it != package_iterator->second->end(); it++) {
            SoundInfo* info = it->second;
            if (SoundType::MUSIC == info->type) {
                m_effects->removeObjectForKey(info->fileName);
                m_sounds->removeObjectForKey(info->fileName);
            }
            CC_SAFE_DELETE(info);
            it->second = nullptr;
        }
    }
}
    
void SoundManager::removeSound(size_t sound_id, size_t package_id) {
    using cocos2d::CCArray;
    using cocos2d::CCObject;
    SoundInfo* info = this->getSoundInfo(sound_id, package_id);
    if (info) {
        if (SoundType::MUSIC == info->type) {
            m_music->removeObjectForKey(info->fileName);
        } else if (SoundType::EFFECT == info->type || SoundType::STREAM_SFX == info->type) {
            m_effects->removeObjectForKey(info->fileName);
        }
        m_sounds->removeObjectForKey(info->fileName);
    }
}
    
AudioSource* SoundManager::getSound(size_t sound_id, size_t package_id) {
    SoundInfo* info = this->getSoundInfo(sound_id, package_id);
    CCObject* objects = m_sounds->objectForKey(info->fileName);
    SoundPool* sound_pool = static_cast<SoundPool*>(objects);
    CCAssert(sound_pool != nullptr, "SoundManager: Sounds can't be found");
    AudioSource* sound = sound_pool->getSound();
    CCAssert(sound != nullptr, "SoundManager: Sound can't be found");
    this->setSoundVolume(info, sound);
    return sound;
}
    
bool SoundManager::hasSound(size_t sound_id, size_t package_id) {
    SoundInfo* info = this->getSoundInfo(sound_id, package_id);
    if (info) {
        return m_sounds->objectForKey(info->fileName);
    }
    return false;
}
    
cocos2d::CCArray* SoundManager::play(size_t sound_id, size_t package_id) {
    using cocos2d::CCArray;
    using cocos2d::CCObject;
    CCObject* object = nullptr;
    SoundInfo* info = this->getSoundInfo(sound_id, package_id);
    CCAssert(info, "SoundManager: SoundInfo can't be found");
    CCObject* objects = m_sounds->objectForKey(info->fileName);
    SoundPool* sound_pool = static_cast<SoundPool*>(objects);
    CCArray* sounds = this->getSoundsByInfo(info);
    CCArray* sounds_to_play = CCArray::create();
    if (!m_isEnabled) return sounds_to_play;
    if (sound_pool->isPaused()) {
        CCARRAY_FOREACH(sounds, object) {
            AudioSource* sound = static_cast<AudioSource*>(object);
            this->setSoundVolume(info, sound);
            sound->play();
            sounds_to_play->addObject(sound);
        }
    } else {
        CCAssert(sound_pool != nullptr, "SoundManager: Sounds can't be found");
        AudioSource* sound = sound_pool->getSound();
        CCAssert(sound != nullptr, "SoundManager: Sound can't be found");
        this->setSoundVolume(info, sound);
        sounds_to_play->addObject(sound);
        sounds->addObject(sound);
        sound->play();
    }
    
    return sounds_to_play;
}
    
cocos2d::CCArray* SoundManager::play(size_t sound_id, size_t package_id, float startTime) {
    using cocos2d::CCArray;
    using cocos2d::CCObject;
    CCObject* object = nullptr;
    SoundInfo* info = this->getSoundInfo(sound_id, package_id);
    CCAssert(info, "SoundManager: SoundInfo can't be found");
    CCObject* objects = m_sounds->objectForKey(info->fileName);
    SoundPool* sound_pool = static_cast<SoundPool*>(objects);
    CCArray* sounds = this->getSoundsByInfo(info);
    CCArray* sounds_to_play = CCArray::create();
    if (!m_isEnabled) return sounds_to_play;
    if (sound_pool->getPaused().size() > 0) {
        CCARRAY_FOREACH(sounds, object) {
            AudioSource* sound = static_cast<AudioSource*>(object);
            this->setSoundVolume(info, sound);
            sound->play(startTime);
            sounds_to_play->addObject(sound);
        }
    } else {
        CCAssert(sound_pool != nullptr, "SoundManager: Sounds can't be found");
        AudioSource* sound = sound_pool->getSound();
        CCAssert(sound != nullptr, "SoundManager: Sound can't be found");
        this->setSoundVolume(info, sound);
        sounds_to_play->addObject(sound);
        sounds->addObject(sound);
        sound->play(startTime);
    }
    return sounds_to_play;
}
    
cocos2d::CCArray* SoundManager::play(size_t sound_id, size_t package_id, float startTime, size_t loops) {
    using cocos2d::CCArray;
    using cocos2d::CCObject;
    CCObject* object = nullptr;
    SoundInfo* info = this->getSoundInfo(sound_id, package_id);
    CCAssert(info, "SoundManager: SoundInfo can't be found");
    CCObject* objects = m_sounds->objectForKey(info->fileName);
    SoundPool* sound_pool = static_cast<SoundPool*>(objects);
    CCAssert(sound_pool != nullptr, "SoundManager: Sounds can't be found");
    CCArray* sounds = this->getSoundsByInfo(info);
    CCArray* sounds_to_play = CCArray::create();
    if (!m_isEnabled) return sounds_to_play;
    if (sound_pool->isPaused()) {
        CCARRAY_FOREACH(sounds, object) {
            AudioSource* sound = static_cast<AudioSource*>(object);
            this->setSoundVolume(info, sound);
            sound->play(startTime, loops);
            sounds_to_play->addObject(sound);
        }
    } else {
            AudioSource* sound = sound_pool->getSound();
            CCAssert(sound != nullptr, "SoundManager: Sound can't be found");
            this->setSoundVolume(info, sound);
            sounds_to_play->addObject(sound);
            sounds->addObject(sound);
            sound->play(startTime, loops);
        }
    return sounds_to_play;
}
    
cocos2d::CCArray* SoundManager::stop(size_t sound_id, size_t package_id) {
    using cocos2d::CCArray;
    using cocos2d::CCObject;
    CCArray* sounds_to_stop = CCArray::create();
    SoundInfo* info = this->getSoundInfo(sound_id, package_id);
    if (info) {
        CCArray* sounds = this->getSoundsByInfo(info);
        while (sounds->count() > 0) {
            CCObject* object = sounds->objectAtIndex(0);
            AudioSource* sound = static_cast<AudioSource*>(object);
            sounds_to_stop->addObject(sound);
            sounds->removeObject(sound);
            sound->stop();
        }
    }
    return sounds_to_stop;
}
    
cocos2d::CCArray* SoundManager::pause(size_t sound_id, size_t package_id) {
    using cocos2d::CCArray;
    using cocos2d::CCObject;
    CCObject* object = nullptr;
    CCArray* sounds_to_pause = CCArray::create();
    SoundInfo* info = this->getSoundInfo(sound_id, package_id);
    if (info) {
        CCArray* sounds = this->getSoundsByInfo(info);
        CCARRAY_FOREACH(sounds, object) {
            AudioSource* sound = static_cast<AudioSource*>(object);
            sounds_to_pause->addObject(sound);
            sound->pause();
        }
    }
    return sounds_to_pause;
}
        
void SoundManager::stopAll(void) {
    this->stopMusic();
    this->stopEffects();
}
    
void SoundManager::pauseAll(void) {
    this->pauseMusic();
    this->pauseEffects();
}
    
void SoundManager::stopEffects(void) {
    using cocos2d::CCArray;
    using cocos2d::CCDictElement;
    CCDictElement* element = nullptr;
    CCDICT_FOREACH(m_effects, element) {
        CCArray* sounds = static_cast<CCArray*>(element->getObject());
        while (sounds->count() > 0) {
            CCObject* object = sounds->objectAtIndex(0);
            AudioSource* sound = static_cast<AudioSource*>(object);
            sound->stop();
            sounds->removeObjectAtIndex(0);
        }
    }
}
    
void SoundManager::stopMusic(void) {
    using cocos2d::CCArray;
    using cocos2d::CCDictElement;
    CCDictElement* element = nullptr;
    CCDICT_FOREACH(m_music, element) {
        CCArray* musics = static_cast<CCArray*>(element->getObject());
        while (musics->count() > 0) {
            CCObject* object = musics->objectAtIndex(0);
            AudioSource* sound = static_cast<AudioSource*>(object);
            sound->stop();
            musics->removeObjectAtIndex(0);
        }
    }
}
    
void SoundManager::pauseEffects(void) {
    using cocos2d::CCArray;
    using cocos2d::CCObject;
    using cocos2d::CCDictElement;
    CCObject* object = nullptr;
    CCDictElement* element = nullptr;
    CCDICT_FOREACH(m_effects, element) {
        CCArray* sounds = static_cast<CCArray*>(element->getObject());
        CCARRAY_FOREACH(sounds, object) {
            AudioSource* sound = static_cast<AudioSource*>(object);
            sound->pause();
        }
    }
}
    
void SoundManager::pauseMusic(void) {
    using cocos2d::CCArray;
    using cocos2d::CCObject;
    using cocos2d::CCDictElement;
    CCObject* object = nullptr;
    CCDictElement* element = nullptr;
    CCDICT_FOREACH(m_music, element) {
        CCArray* musics = static_cast<CCArray*>(element->getObject());
        CCARRAY_FOREACH(musics, object) {
            AudioSource* sound = static_cast<AudioSource*>(object);
            sound->pause();
        }
    }
}
    
void SoundManager::resumeAll(void) {
    this->resumeMusic();
    this->resumeEffects();
}
    
void SoundManager::resumeEffects(void) {
    using cocos2d::CCArray;
    using cocos2d::CCObject;
    using cocos2d::CCDictElement;
    if (!m_isEnabled) return;
    CCObject* object = nullptr;
    CCDictElement* element = nullptr;
    CCDICT_FOREACH(m_effects, element) {
        CCArray* effects = static_cast<CCArray*>(element->getObject());
        CCARRAY_FOREACH(effects, object) {
            AudioSource* sound = static_cast<AudioSource*>(object);
            CCLOG("%d", sound->getStatus());
            if (AudioSource::PAUSED == sound->getStatus()) {
                sound->setVolume(m_effectsVolume);
                sound->play();
            }
        }
    }
}
    
void SoundManager::resumeMusic(void) {
    using cocos2d::CCArray;
    using cocos2d::CCObject;
    using cocos2d::CCDictElement;
    if (!m_isEnabled) return;
    CCObject* object = nullptr;
    CCDictElement* element = nullptr;
    CCDICT_FOREACH(m_music, element) {
        CCArray* musics = static_cast<CCArray*>(element->getObject());
        CCARRAY_FOREACH(musics, object) {
            AudioSource* sound = static_cast<AudioSource*>(object);
            if (AudioSource::PAUSED == sound->getStatus()) {
                sound->setVolume(m_musicVolume);
                sound->play();
            }
        }
    }
}
    
bool SoundManager::isPlaying(void) {
    using cocos2d::CCArray;
    using cocos2d::CCDictElement;
    SoundPool* pool = nullptr;
    CCDictElement* element = nullptr;
    CCDICT_FOREACH(m_sounds, element) {
        pool = static_cast<SoundPool*>(element->getObject());
        if (pool->isPlaying()) return true;
    }
    return false;
}
    
bool SoundManager::isPlaying(size_t sound_id, size_t package_id) {
    using cocos2d::CCObject;
    SoundInfo* info = this->getSoundInfo(sound_id, package_id);
    if (!info) return false;
    CCObject* objects = m_sounds->objectForKey(info->fileName);
    SoundPool* sound_pool = static_cast<SoundPool*>(objects);
    return sound_pool->isPlaying();
}
    
bool SoundManager::isPaused(size_t sound_id, size_t package_id) {
    using cocos2d::CCObject;
    SoundInfo* info = this->getSoundInfo(sound_id, package_id);
    if (!info) return false;
    CCObject* objects = m_sounds->objectForKey(info->fileName);
    SoundPool* sound_pool = static_cast<SoundPool*>(objects);
    return sound_pool->isPaused();
}
    
void SoundManager::stopPackage(size_t package_id) {
    SoundInfo* info = nullptr;
    InfoMap* info_map = m_packages[package_id];
    if (info_map) {
        for (InfoMap::iterator it = info_map->begin(); it != info_map->end(); it++) {
            info = it->second;
            this->stop(info->soundId, package_id);
        }
    }
}
    
void SoundManager::pausePackage(size_t package_id) {
    SoundInfo* info = nullptr;
    InfoMap* info_map = m_packages[package_id];
    if (info_map) {
        for (InfoMap::iterator it = info_map->begin(); it != info_map->end(); it++) {
            info = it->second;
            this->pause(info->soundId, package_id);
        }
    }
}
    
void SoundManager::resumePackage(size_t package_id) {
    SoundInfo* info = nullptr;
    InfoMap* info_map = m_packages[package_id];
    if (info_map) {
        for (InfoMap::iterator it = info_map->begin(); it != info_map->end(); it++) {
            info = it->second;
            if (this->isPaused(info->soundId, package_id))
                this->pause(info->soundId, package_id);
        }
    }
}
    
void SoundManager::setVolume(float value) {
    this->setMusicVolume(value);
    this->setEffectsVolume(value);
}
    
float SoundManager::getMusicVolume(void) {
    return m_musicVolume;
}
    
void SoundManager::setMusicVolume(float value) {
    using cocos2d::CCArray;
    using cocos2d::CCObject;
    using cocos2d::CCDictElement;
    m_musicVolume = value;
    CCObject* object = nullptr;
    CCDictElement* element = nullptr;
    CCDICT_FOREACH(m_music, element) {
        CCArray* musics = static_cast<CCArray*>(element->getObject());
        CCARRAY_FOREACH(musics, object) {
            AudioSource* sound = static_cast<AudioSource*>(object);
            sound->setVolume(value);
        }
    }
}
    
float SoundManager::getEffectsVolume(void) {
    return m_effectsVolume;
}
    
void SoundManager::setEffectsVolume(float value) {
    using cocos2d::CCArray;
    using cocos2d::CCObject;
    using cocos2d::CCDictElement;
    m_effectsVolume = value;
    CCObject* object = nullptr;
    CCDictElement* element = nullptr;
    CCDICT_FOREACH(m_effects, element) {
        CCArray* effects = static_cast<CCArray*>(element->getObject());
        CCARRAY_FOREACH(effects, object) {
            AudioSource* sound = static_cast<AudioSource*>(object);
            sound->setVolume(value);
        }
    }
}
    
void SoundManager::setEnabled(bool value) {
    m_isEnabled = value;
    if (!m_isEnabled) {
        this->stopAll();
    }
}
    
bool SoundManager::isEnabled(void) {
    return m_isEnabled;
}
    
void SoundManager::setSoundVolume(SoundInfo* info, AudioSource* sound) {
    if (SoundType::EFFECT == info->type || SoundType::STREAM_SFX == info->type) {
        sound->setVolume(m_effectsVolume);
    } else if (SoundType::MUSIC == info->type) {
        sound->setVolume(m_musicVolume);
    }
}
    
cocos2d::CCArray* SoundManager::getSoundsByInfo(SoundInfo *info) {
    using cocos2d::CCArray;
    using cocos2d::CCObject;
    CCArray* sounds = nullptr;
    if (SoundType::EFFECT == info->type || SoundType::STREAM_SFX == info->type) {
        sounds = static_cast<CCArray*>(m_effects->objectForKey(info->fileName));
    } else if (SoundType::MUSIC == info->type) {
        sounds = static_cast<CCArray*>(m_music->objectForKey(info->fileName));
    }
    return sounds;
}
    
SoundManager::SoundInfo* SoundManager::getSoundInfo(size_t sound_id, size_t package_id) {
    using cocos2d::CCObject;
    SoundInfo* info = nullptr;
    InfoMap* info_map = m_packages[package_id];
    if (info_map) {
        info = (*info_map)[sound_id];
    }
    return info;
}

}