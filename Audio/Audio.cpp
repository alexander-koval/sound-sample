//
//  Audio.cpp
//  HelloWorld
//
//  Created by Александр Коваль on 23.04.14.
//
//

#include "Audio.h"
#include "CCDirector.h"
#include "platform/CCCommon.h"
#include "actions/CCActionManager.h"
#include "actions/CCAction.h"
#include "CCScheduler.h"
#include "ccMacros.h"

using namespace cocos2d;

namespace cribys {
AudioDevice* g_audioDevice = new AudioDevice();

AudioSource::AudioSource()
: CCObject()
, m_source(0)
, m_fileName(nullptr)
, m_isStreaming(false)
, m_selectorTarget(nullptr) {
    AL_CHECK_ERRORS(alGenSources(1, &m_source));
    AL_CHECK_ERRORS(alSourcei(m_source, AL_BUFFER, 0));
    cocos2d::CCDirector* director = cocos2d::CCDirector::sharedDirector();
    m_actionManager = director->getActionManager();
    m_actionManager->retain();
    m_scheduler = director->getScheduler();
    m_scheduler->retain();
}

AudioSource::~AudioSource() {
    AL_CHECK_ERRORS(alDeleteSources(1, &m_source));
    CC_SAFE_RELEASE(m_actionManager);
    m_actionManager = nullptr;
    if (m_selectorTarget)
        CC_SAFE_RELEASE(m_selectorTarget);
}
    
void AudioSource::setCallback(cocos2d::CCObject *selectorTarget, cocos2d::SEL_CallFuncO selector) {
    if (selectorTarget) selectorTarget->retain();
    if (m_selectorTarget) m_selectorTarget->release();
    m_selectorTarget = selectorTarget;
    m_callback = selector;
}
    
void AudioSource::setFileName(const char *name) {
    m_fileName = name;
}
    
const char* AudioSource::getFileName(void) const {
    return m_fileName;
}
    
bool AudioSource::isPlaying(void) const {
    return (this->getStatus() == PLAYING);
}
    
bool AudioSource::isStreaming(void) const {
    return m_isStreaming;
}
    
void AudioSource::setActionManager(cocos2d::CCActionManager *action_manager) {
    if (action_manager != m_actionManager) {
        this->stopAllActions();
        CC_SAFE_RETAIN(action_manager);
        CC_SAFE_RELEASE(m_actionManager);
        m_actionManager = action_manager;
    }
}
    
cocos2d::CCActionManager* AudioSource::getActionManager(void) {
    return m_actionManager;
}
    
cocos2d::CCAction* AudioSource::runAction(cocos2d::CCAction *action) {
    CCAssert(action != nullptr, "Argument must be non-nil");
//    m_actionManager->addAction(action, this, false);
    return action;
}
    
void AudioSource::stopAllActions(void) {
    m_actionManager->removeAllActionsFromTarget(this);
}
    
void AudioSource::stopAction(cocos2d::CCAction *action) {
    m_actionManager->removeAction(action);
}
    
cocos2d::CCAction* AudioSource::getActionByTag(int tag) {
    CCAssert( tag != kCCActionTagInvalid, "Invalid tag");
    return m_actionManager->getActionByTag(tag, this);
}
    
unsigned int AudioSource::numberOfRunningActions(void) {
    return m_actionManager->numberOfRunningActionsInTarget(this);
}
    
AudioTime AudioSource::getElapsedTime(void) const {
    ALint offset;
    AL_CHECK_ERRORS(alGetSourcei(m_source, AL_SEC_OFFSET, &offset));
    return AudioTime(static_cast<float>(offset));
}

void AudioSource::setRelative(bool relative) {
    AL_CHECK_ERRORS(alSourcei(m_source, AL_SOURCE_RELATIVE, relative));
}

bool AudioSource::isRelative(void) const {
    ALint relative;
    AL_CHECK_ERRORS(alGetSourcei(m_source, AL_SOURCE_RELATIVE, &relative));
    return relative;
}

void AudioSource::setVolume(float volume) {
    AL_CHECK_ERRORS(alSourcef(m_source, AL_GAIN, volume));
}

float AudioSource::getVolume(void) const {
    ALfloat gain;
    AL_CHECK_ERRORS(alGetSourcef(m_source, AL_GAIN, &gain));
    return gain;
}
    
void AudioSource::setSpeed(float speed) {
    alSourcef(m_source, AL_PITCH, speed);
}
    
float AudioSource::getSpeed(void) const {
    ALfloat pitch;
    AL_CHECK_ERRORS(alGetSourcef(m_source, AL_PITCH, &pitch));
    return pitch;
}
    
void AudioSource::setFade(float fade) {
    AL_CHECK_ERRORS(alSourcef(m_source, AL_ROLLOFF_FACTOR, fade));
}
    
float AudioSource::getFade(void) {
    ALfloat fade;
    AL_CHECK_ERRORS((alGetSourcef(m_source, AL_ROLLOFF_FACTOR, &fade)));
    return fade;
}
    
AudioSource::Status AudioSource::getStatus(void) const {
    ALint status;
    AL_CHECK_ERRORS(alGetSourcei(m_source, AL_SOURCE_STATE, &status));
    switch (status) {
        case AL_INITIAL :
        case AL_STOPPED : return STOPPED;
        case AL_PAUSED :  return PAUSED;
        case AL_PLAYING : return PLAYING;
    }
    return STOPPED;
}

int Audio::getFormatFromChannelCount(unsigned int channelCount) {
    int format = 0;
    switch (channelCount) {
        case 1  : format = AL_FORMAT_MONO16;                    break;
        case 2  : format = AL_FORMAT_STEREO16;                  break;
        case 4  : format = alGetEnumValue("AL_FORMAT_QUAD16");  break;
        case 6  : format = alGetEnumValue("AL_FORMAT_51CHN16"); break;
        case 7  : format = alGetEnumValue("AL_FORMAT_61CHN16"); break;
        case 8  : format = alGetEnumValue("AL_FORMAT_71CHN16"); break;
        default : format = 0;                                   break;
    }
    return format;
}

const char* Audio::getError(void) {
    ALenum errorCode = alGetError();
    if (errorCode != AL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
            case AL_INVALID_NAME :
                error = "AL_INVALID_NAME: unacceptable name has been specified";
                break;
            case AL_INVALID_ENUM :
                error = "AL_INVALID_ENUM: unacceptable value has been specified for an enumerated argument";
                break;
            case AL_INVALID_VALUE :
                error = "AL_INVALID_VALUE: numeric argument is out of range";
                break;
            case AL_INVALID_OPERATION :
                error = "AL_INVALID_OPERATION: the specified operation is not allowed in the current state";
                break;
            case AL_OUT_OF_MEMORY :
                error = "AL_OUT_OF_MEMORY: there is not enough memory left to execute the command";
                break;
        }
        return error.c_str();
    }
    return nullptr;
}

AudioDevice::AudioDevice()
: CCObject()
, m_device(nullptr)
, m_context(nullptr) {
    m_device = alcOpenDevice(nullptr);
    CCAssert(m_device, std::string("Failed to open audio device: ").append(Audio::getError()).c_str());
    m_context = alcCreateContext(m_device, nullptr);
    CCAssert(m_context, std::string("Failed to create audio context: ").append(Audio::getError()).c_str());
    alcMakeContextCurrent(m_context);
}

AudioDevice::~AudioDevice() {
    alcMakeContextCurrent(nullptr);
    if (m_context) alcDestroyContext(m_context);
    if (m_device) alcCloseDevice(m_device);
}
    
AudioTime::AudioTime()
: m_microseconds(0) {}
    
AudioTime::AudioTime(int64_t microseconds)
: m_microseconds(microseconds) {}
    
AudioTime::AudioTime(int32_t milliseconds)
: m_microseconds(0) {
    m_microseconds = static_cast<int64_t>(milliseconds * 1000);
}
    
AudioTime::AudioTime(float seconds)
: m_microseconds(0) {
    m_microseconds = static_cast<int64_t>(seconds * 1000000.f);
}
    
float AudioTime::asSeconds(void) const {
    return m_microseconds / 1000000.f;
}
    
int32_t AudioTime::asMilliseconds(void) const {
    return static_cast<int32_t>(m_microseconds / 1000);
}
    
int64_t AudioTime::asMicroseconds(void) const {
    return m_microseconds;
}
}


