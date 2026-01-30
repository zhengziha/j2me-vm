#include "AudioManager.hpp"
#include "Logger.hpp"
#include <sstream>
#include <cstring>
#include <algorithm>

namespace j2me {
namespace core {

// 静态成员初始化
std::unique_ptr<AudioManager> AudioManager::instance = nullptr;
std::mutex AudioManager::instanceMutex;

AudioManager::AudioManager()
    : initialized(false)
    , audioDevice(0)
    , currentBufferPos(0)
    , volume(100)
    , paused(false)
{
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

    // 启动音频设备（暂停状态）
    SDL_PauseAudioDevice(audioDevice, 1);

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
        // 停止并关闭音频设备
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
        currentBuffer.clear();
        currentBufferPos = 0;
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

    // 将音频数据加入队列
    {
        std::lock_guard<std::mutex> bufferLock(bufferMutex);
        audioBufferQueue.push(audioData);

        std::stringstream ss;
        ss << "Audio data queued for playback: " << audioData.size() << " bytes, "
           << "sampleRate=" << sampleRate << ", channels=" << channels << ", sampleSize=" << sampleSize;
        LOG_INFO(ss.str().c_str());
    }

    // 如果设备处于暂停状态，恢复播放
    if (paused) {
        resumeAudio();
    }

    return true;
}

void AudioManager::stopAudio() {
    std::lock_guard<std::mutex> bufferLock(bufferMutex);

    // 清空缓冲区队列
    while (!audioBufferQueue.empty()) {
        audioBufferQueue.pop();
    }

    currentBuffer.clear();
    currentBufferPos = 0;

    // 暂停音频设备
    if (audioDevice != 0) {
        SDL_LockAudioDevice(audioDevice);
        SDL_PauseAudioDevice(audioDevice, 1);
        SDL_UnlockAudioDevice(audioDevice);
        paused = true;
    }

    LOG_INFO("Audio playback stopped");
}

void AudioManager::pauseAudio() {
    if (!initialized || paused) {
        return;
    }

    if (audioDevice != 0) {
        SDL_LockAudioDevice(audioDevice);
        SDL_PauseAudioDevice(audioDevice, 1);
        SDL_UnlockAudioDevice(audioDevice);
        paused = true;
    }

    LOG_INFO("Audio playback paused");
}

void AudioManager::resumeAudio() {
    if (!initialized || !paused) {
        return;
    }

    if (audioDevice != 0) {
        SDL_LockAudioDevice(audioDevice);
        SDL_PauseAudioDevice(audioDevice, 0);
        SDL_UnlockAudioDevice(audioDevice);
        paused = false;
    }

    LOG_INFO("Audio playback resumed");
}

bool AudioManager::setVolume(int vol) {
    // 确保音量在0-100之间
    int clampedVolume = std::max(0, std::min(100, vol));

    std::lock_guard<std::mutex> bufferLock(bufferMutex);
    volume = clampedVolume;

    std::stringstream ss;
    ss << "Audio volume set to: " << volume << "%";
    LOG_INFO(ss.str().c_str());
    return true;
}

int AudioManager::getVolume() const {
    return volume;
}

bool AudioManager::isPlaying() const {
    if (!initialized || paused) {
        return false;
    }

    std::lock_guard<std::mutex> bufferLock(const_cast<std::mutex&>(bufferMutex));
    return !audioBufferQueue.empty() || !currentBuffer.empty();
}

bool AudioManager::isPaused() const {
    return paused;
}

bool AudioManager::isInitialized() const {
    return initialized;
}

void AudioManager::audioCallback(void* userdata, uint8_t* stream, int len) {
    AudioManager* manager = static_cast<AudioManager*>(userdata);
    manager->playAudioData(stream, len);
}

void AudioManager::applyVolume(uint8_t* stream, int len) {
    if (volume >= 100) {
        return; // 最大音量，不需要调整
    }

    float volumeFactor = static_cast<float>(volume) / 100.0f;
    int16_t* samples = reinterpret_cast<int16_t*>(stream);
    int sampleCount = len / sizeof(int16_t);

    for (int i = 0; i < sampleCount; i++) {
        samples[i] = static_cast<int16_t>(samples[i] * volumeFactor);
    }
}

void AudioManager::playAudioData(uint8_t* stream, int len) {
    // 清空输出缓冲区
    std::memset(stream, 0, len);

    // 如果已暂停，返回静音
    if (paused) {
        return;
    }

    std::lock_guard<std::mutex> bufferLock(bufferMutex);

    int bytesWritten = 0;

    // 首先处理当前缓冲区
    while (bytesWritten < len) {
        // 如果当前缓冲区为空，从队列中获取下一个
        if (currentBuffer.empty() || currentBufferPos >= currentBuffer.size()) {
            if (audioBufferQueue.empty()) {
                // 没有更多数据了
                break;
            }

            currentBuffer = audioBufferQueue.front();
            audioBufferQueue.pop();
            currentBufferPos = 0;
        }

        // 计算可复制的字节数
        int bytesAvailable = currentBuffer.size() - currentBufferPos;
        int bytesToCopy = std::min(len - bytesWritten, bytesAvailable);

        if (bytesToCopy > 0) {
            // 复制音频数据到输出缓冲区
            std::memcpy(stream + bytesWritten, currentBuffer.data() + currentBufferPos, bytesToCopy);
            currentBufferPos += bytesToCopy;
            bytesWritten += bytesToCopy;
        }
    }

    // 应用音量
    if (bytesWritten > 0) {
        applyVolume(stream, bytesWritten);
    }
}

} // namespace core
} // namespace j2me
