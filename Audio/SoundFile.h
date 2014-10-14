//
//  SoundFile.h
//  HelloWorld
//
//  Created by Александр Коваль on 25.04.14.
//
//

#ifndef __HelloWorld__SoundFile__ 
#define __HelloWorld__SoundFile__

#include <sndfile.h>
#include "cocoa/CCObject.h"

namespace cribys {
class SoundFile : public cocos2d::CCObject {
public:
    static SoundFile* create(const char* pszFileName);
    
    SoundFile();
    
    ~SoundFile();
    
    bool initWithFile(const char* strPath);
    
    std::size_t read(int16_t* data, std::size_t sampleCount);
    
    void seek(float second);
    
    const unsigned int getChannelCount(void) const;
    
    const unsigned int getSampleRate(void) const;
    
    const std::size_t getSampleCount(void) const;
    
    const int64_t getFrames(void) const;
    
    const SF_FORMAT_INFO& getFormat(void) const;
    
    const SF_FORMAT_INFO& getSubFormat(void) const;
    
    const char* getName(void) const;
    
    unsigned long getMemorySize(void) const;
    
protected:
    struct MemoryInfos {
        unsigned char* dataStart;
        unsigned char* dataPtr;
        sf_count_t  totalSize;
    };
    SNDFILE* m_file;
    const char* m_name;
    MemoryInfos m_memory;
    SF_FORMAT_INFO m_format;
    SF_FORMAT_INFO m_subFormat;
    std::size_t  m_sampleCount;
    unsigned int m_channelCount;
    unsigned int m_sampleRate;
    int64_t m_frames;
    
private:
    static sf_count_t getMemoryLength(void* user_data);
    static sf_count_t readMemory(void* ptr, sf_count_t count, void* user_data);
    static sf_count_t seekMemory(sf_count_t offset, int whence, void* user_data);
    static sf_count_t tellMemory(void* user_data);
    static sf_count_t writeMemory(const void* ptr, sf_count_t count, void* user_data);
};
};

#endif /* defined(__HelloWorld__SoundFile__) */
