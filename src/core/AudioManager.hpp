#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include <mutex>
#include <queue>

namespace j2me {
namespace core {

class AudioManager {
private:
    static std::unique_ptr<AudioManager> instance;
    static std::mutex instanceMutex;
    
    bool initialized;
    SDL_AudioDeviceID audioDevice;
    std::mutex audioMutex;
    
    // 音频缓冲区队列
    std::queue<std::vector<uint8_t>> audioBufferQueue;
    std::mutex bufferMutex;
    
    // 音频回调函数
    static void audioCallback(void* userdata, uint8_t* stream, int len);
    
    // 播放音频数据
    void playAudioData(uint8_t* stream, int len);
    
public:
    AudioManager();
    ~AudioManager();
    
    // 获取单例实例
    static AudioManager& getInstance();
    
    // 初始化音频系统
    bool init();
    
    // 关闭音频系统
    void shutdown();
    
    // 播放音频数据
    bool playAudio(const std::vector<uint8_t>& audioData, int sampleRate, int channels, int sampleSize);
    
    // 停止播放
    void stopAudio();
    
    // 设置音量
    bool setVolume(int volume);
    
    // 检查是否初始化
    bool isInitialized() const;
};

} // namespace core
} // namespace j2me
