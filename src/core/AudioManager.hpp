#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include <mutex>
#include <queue>
#include <vector>
#include <cstddef>

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

    // 当前播放位置（用于部分播放的缓冲区）
    size_t currentBufferPos;
    std::vector<uint8_t> currentBuffer;

    // 音频参数
    int volume; // 0-100
    bool paused;

    // 音频回调函数
    static void audioCallback(void* userdata, uint8_t* stream, int len);

    // 播放音频数据
    void playAudioData(uint8_t* stream, int len);

    // 应用音量
    void applyVolume(uint8_t* stream, int len);

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

    // 停止播放并清空队列
    void stopAudio();

    // 暂停播放
    void pauseAudio();

    // 恢复播放
    void resumeAudio();

    // 设置音量 (0-100)
    bool setVolume(int volume);

    // 获取音量
    int getVolume() const;

    // 检查是否正在播放
    bool isPlaying() const;

    // 检查是否已暂停
    bool isPaused() const;

    // 检查是否初始化
    bool isInitialized() const;
};

} // namespace core
} // namespace j2me
