//
//  SoundManager.h
//  SoundEngine
//
//  Created by Александр Коваль on 09.05.14.
//  Copyright (c) 2014 Александр Коваль. All rights reserved.
//

#ifndef __SoundEngine__SoundManager__
#define __SoundEngine__SoundManager__

#include <map>
#include <vector>
#include "Audio.h"
#include "cocoa/CCObject.h"

namespace cocos2d { class CCDictionary; }
namespace cribys {
class SoundPool : public cocos2d::CCObject {
public:
    static SoundPool* create();
    
    explicit SoundPool();
        
    virtual ~SoundPool(void);
        
    AudioSource* getSound(void);
        
    void putSound(AudioSource* sound);
    
    bool isPlaying(void);
    
    bool isPaused(void);
    
    std::vector<AudioSource*> getPlaying(void);
    
    std::vector<AudioSource*> getPaused(void);
    
    void stopAll(void);
    
    void pauseAll(void);
    
    size_t getSize(void);
    
    void setSize(size_t size);
    
private:
    std::vector<AudioSource*> m_pool;
};

class SoundManager : public cocos2d::CCObject {
public:
   
    
    enum class SoundType {
        MUSIC       = 0,
        EFFECT      = 1,
        STREAM_SFX  = 2,
    };
    
    class PlayMode {
    public:
        static const PlayMode MULTIPLE(size_t count);
        static const PlayMode SINGLE;
        size_t getCount(void) const;
    private:
        PlayMode(size_t count = 1);
        size_t m_count;
    };
    
    static SoundManager* sharedSoundManager(void);
    
    static void end(void);
    
    SoundManager(void);
    
    virtual ~SoundManager(void);
    
    bool addSoundInfo(size_t sound_id,
                      size_t package_id,
                      const char* file_name,
                      SoundType type,
                      PlayMode mode);
    
    void removeAll();
    
    void removeEffects();
    
    void removeMusic();

    void removeSound(size_t sound_id, size_t package_id);
    
    cocos2d::CCArray* play(size_t sound_id, size_t package_id);
    
    cocos2d::CCArray* play(size_t sound_id, size_t package_id, float startTime);
    
    cocos2d::CCArray* play(size_t sound_id, size_t package_id, float startTime, size_t loops);
    
    cocos2d::CCArray* stop(size_t sound_id, size_t package_id);
    
    cocos2d::CCArray* pause(size_t sound_id, size_t package_id);
    
    AudioSource* getSound(size_t sound_id, size_t package_id);

    bool hasSound(size_t sound_id, size_t package_id);
    
    void stopAll(void);
    
    void pauseAll(void);
    
    void stopEffects(void);
    
    void stopMusic(void);
    
    void pauseEffects(void);
    
    void pauseMusic(void);
    
    void resumeAll(void);
    
    void resumeEffects(void);
    
    void resumeMusic(void);
    
    bool isPlaying(void);
    
    void stopPackage(size_t package_id);
    
    void pausePackage(size_t package_id);
    
    void resumePackage(size_t package_id);

    bool isPlaying(size_t sound_id, size_t package_id);
    
    bool isPaused(size_t sound_id, size_t package_id);
    
    void setVolume(float value);
    
    void setMusicVolume(float value);
    
    void setEffectsVolume(float value);
    
    float getMusicVolume(void);
    
    float getEffectsVolume(void);
    
    void setEnabled(bool value);
    
    bool isEnabled(void);
    
protected:
    struct SoundInfo {
        SoundInfo(size_t package_id,
                  size_t sound_id,
                  const char* file_name,
                  SoundType sound_type,
                  PlayMode play_mode)
        : packageId(package_id)
        , soundId(sound_id)
        , fileName(file_name)
        , type(sound_type)
        , mode(play_mode) {};
        
        size_t packageId;
        size_t soundId;
        std::string fileName;
        SoundType type;
        PlayMode mode;
    };

    cocos2d::CCArray* getSoundsByInfo(SoundInfo* info);
    
    SoundInfo* getSoundInfo(size_t sound_id, size_t package_id);
    
    void setSoundVolume(SoundInfo* info, AudioSource* sound);

    typedef std::map<size_t, SoundInfo*> InfoMap;
    std::map<size_t, InfoMap*> m_packages;
    cocos2d::CCDictionary* m_sounds;
    cocos2d::CCDictionary* m_music;
    cocos2d::CCDictionary* m_effects;
    bool m_isEnabled;
    float m_musicVolume;
    float m_effectsVolume;
    
private:
    static SoundManager* s_sharedSoundManager;
};

}

#endif /* defined(__SoundEngine__SoundManager__) */
