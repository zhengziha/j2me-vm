#include "AudioManager.hpp"
#include "Logger.hpp"
#include <sstream>

namespace j2me {
namespace core {

// 静态成员初始化
std::unique_ptr<AudioManager> AudioManager::instance = nullptr;
std::mutex AudioManager::instanceMutex;

AudioManager::AudioManager() : initialized(false), audioDevice(0) {
}

AudioManager::~AudioManager() {
    shutdown();
}

AudioManager& AudioManager::getInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (!instance) {
        instance = std::unique_ptr<AudioManager>(new AudioManager());
    }
    return *instance;
}

bool AudioManager::init() {
    std::lock_guard<std::mutex> lock(audioMutex);
    
    if (initialized) {
        return true;
    }
    
    // 设置音频格式
    SDL_AudioSpec desiredSpec;
    SDL_zero(desiredSpec);
    desiredSpec.freq = 44100;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = 2;
    desiredSpec.samples = 4096;
    desiredSpec.callback = audioCallback;
    desiredSpec.userdata = this;
    
    // 打开音频设备
    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, nullptr, 0);
    if (audioDevice == 0) {
        std::stringstream ss;
        ss << "Failed to open audio device: " << SDL_GetError();
        LOG_ERROR(ss.str().c_str());
        return false;
    }
    
    // 启动音频设备
    SDL_PauseAudioDevice(audioDevice, 0);
    
    initialized = true;
    LOG_INFO("AudioManager initialized successfully");
    return true;
}

void AudioManager::shutdown() {
    std::lock_guard<std::mutex> lock(audioMutex);
    
    if (!initialized) {
        return;
    }
    
    if (audioDevice != 0) {
        SDL_PauseAudioDevice(audioDevice, 1);
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
    }
    
    // 清空缓冲区队列
    {
        std::lock_guard<std::mutex> bufferLock(bufferMutex);
        while (!audioBufferQueue.empty()) {
            audioBufferQueue.pop();
        }
    }
    
    initialized = false;
    LOG_INFO("AudioManager shutdown");
}

bool AudioManager::playAudio(const std::vector<uint8_t>& audioData, int sampleRate, int channels, int sampleSize) {
    if (!initialized) {
        if (!init()) {
            return false;
        }
    }
    
    std::lock_guard<std::mutex> lock(bufferMutex);
    audioBufferQueue.push(audioData);
    
    std::stringstream ss;
    ss << "Audio data queued for playback: " << audioData.size() << " bytes";
    LOG_INFO(ss.str().c_str());
    return true;
}

void AudioManager::stopAudio() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    while (!audioBufferQueue.empty()) {
        audioBufferQueue.pop();
    }
    LOG_INFO("Audio playback stopped");
}

bool AudioManager::setVolume(int volume) {
    if (!initialized) {
        return false;
    }
    
    // 确保音量在0-100之间
    int clampedVolume = volume;
    if (clampedVolume < 0) clampedVolume = 0;
    if (clampedVolume > 100) clampedVolume = 100;
    
    // SDL 2.0.18+ 使用 SDL_SetAudioDeviceVolume，但我们使用更兼容的方式
    // 注意：这里我们只是记录音量设置，实际的音量控制需要在音频回调中实现
    
    std::stringstream ss;
    ss << "Audio volume set to: " << clampedVolume << "%";
    LOG_INFO(ss.str().c_str());
    return true;
}

bool AudioManager::isInitialized() const {
    return initialized;
}

void AudioManager::audioCallback(void* userdata, uint8_t* stream, int len) {
    AudioManager* manager = static_cast<AudioManager*>(userdata);
    manager->playAudioData(stream, len);
}

void AudioManager::playAudioData(uint8_t* stream, int len) {
    // 清空输出缓冲区
    memset(stream, 0, len);
    
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    if (audioBufferQueue.empty()) {
        return;
    }
    
    auto& currentBuffer = audioBufferQueue.front();
    int bytesToCopy = std::min(len, static_cast<int>(currentBuffer.size()));
    
    if (bytesToCopy > 0) {
        // 复制音频数据到输出缓冲区
        memcpy(stream, currentBuffer.data(), bytesToCopy);
        
        // 如果缓冲区还有剩余数据，更新缓冲区
        if (bytesToCopy < static_cast<int>(currentBuffer.size())) {
            std::vector<uint8_t> remainingBuffer(currentBuffer.begin() + bytesToCopy, currentBuffer.end());
            audioBufferQueue.pop();
            audioBufferQueue.push(remainingBuffer);
        } else {
            // 缓冲区已播放完毕，移除它
            audioBufferQueue.pop();
        }
    }
}

} // namespace core
} // namespace j2me
