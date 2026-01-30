#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace j2me {
namespace core {

// 前向声明
class MIDIPlayer;

// 音频资源类型
enum AudioType {
    AUDIO_UNKNOWN,
    AUDIO_WAV,
    AUDIO_MIDI
};

// 音频资源
struct AudioResource {
    std::vector<uint8_t> data;
    int sampleRate;
    int channels;
    int bitsPerSample;
    std::string name;
    AudioType type;

    AudioResource() : sampleRate(44100), channels(2), bitsPerSample(16), type(AUDIO_UNKNOWN) {}
};

// 音频播放通道（用于同时播放多个音效）
struct AudioChannel {
    std::vector<uint8_t> data;
    size_t position;
    bool active;
    int volume; // 0-100
    bool loop;

    AudioChannel() : position(0), active(false), volume(100), loop(false) {}
};

// 背景音乐播放器
class BGMPlayer {
public:
    BGMPlayer();
    ~BGMPlayer();

    bool load(const std::string& name, const AudioResource& resource);
    void play();
    void stop();
    void pause();
    void resume();
    bool isPlaying() const;
    void setVolume(int volume);
    int getVolume() const;
    void setLoop(bool loop);

    // 获取音频数据供回调使用
    bool getAudioData(uint8_t* stream, int len);

private:
    std::mutex mutex;

    // WAV 播放
    AudioResource currentBGM;
    size_t position;
    bool playing;
    bool paused;
    int volume;
    bool loop;

    // MIDI 播放
    std::unique_ptr<MIDIPlayer> midiPlayer;
    bool isMIDI;
};

// 音效播放器
class SFXPlayer {
public:
    SFXPlayer();
    ~SFXPlayer();

    // 播放音效，返回通道ID
    int play(const AudioResource& resource, bool loop = false);
    void stop(int channelId);
    void stopAll();
    void setVolume(int channelId, int volume);
    bool isActive(int channelId) const;

    // 混合所有活跃通道的音频
    void mixAudio(uint8_t* stream, int len);

private:
    std::mutex mutex;
    std::vector<AudioChannel> channels;
    static const int MAX_CHANNELS = 16;
};

// 声音管理器（主入口）
class SoundManager {
private:
    static std::unique_ptr<SoundManager> instance;
    static std::mutex instanceMutex;

    SDL_AudioDeviceID audioDevice;
    bool initialized;
    std::mutex audioMutex;

    BGMPlayer bgmPlayer;
    SFXPlayer sfxPlayer;

    // 音频资源缓存
    std::unordered_map<std::string, AudioResource> resourceCache;
    std::mutex resourceMutex;

    // 音频回调函数
    static void audioCallback(void* userdata, uint8_t* stream, int len);

    // 应用音量
    void applyVolume(uint8_t* stream, int len, int volume);

public:
    SoundManager();
    ~SoundManager();

    static SoundManager& getInstance();

    bool init();
    void shutdown();
    bool isInitialized() const;

    // 资源管理
    bool loadWAV(const std::string& name, const std::vector<uint8_t>& wavData);
    bool loadMIDI(const std::string& name, const std::vector<uint8_t>& midData);
    bool loadWAVFromFile(const std::string& name, const std::string& filePath);
    const AudioResource* getResource(const std::string& name) const;
    void unloadResource(const std::string& name);
    void clearCache();

    // BGM 控制
    bool playBGM(const std::string& name, bool loop = true);
    void stopBGM();
    void pauseBGM();
    void resumeBGM();
    bool isBGMPlaying() const;
    void setBGMVolume(int volume);
    int getBGMVolume() const;

    // SFX 控制
    int playSFX(const std::string& name, bool loop = false);
    void stopSFX(int channelId);
    void stopAllSFX();
    void setSFXVolume(int channelId, int volume);

    // 全局音量
    void setMasterVolume(int volume);
    int getMasterVolume() const;

private:
    int masterVolume;
};

// WAV 文件加载器
class WAVLoader {
public:
    // 解析 WAV 文件数据
    static bool parse(const std::vector<uint8_t>& wavData, AudioResource& resource);

    // 从文件加载
    static bool loadFromFile(const std::string& filePath, AudioResource& resource);

private:
    // WAV 文件头结构
    #pragma pack(push, 1)
    struct WAVHeader {
        char riff[4];           // "RIFF"
        uint32_t fileSize;      // 文件大小 - 8
        char wave[4];           // "WAVE"
        char fmt[4];            // "fmt "
        uint32_t fmtSize;       // fmt chunk 大小
        uint16_t audioFormat;   // 音频格式 (1 = PCM)
        uint16_t numChannels;   // 声道数
        uint32_t sampleRate;    // 采样率
        uint32_t byteRate;      // 字节率
        uint16_t blockAlign;    // 块对齐
        uint16_t bitsPerSample; // 位深度
    };
    #pragma pack(pop)

    // 查找 chunk
    static const uint8_t* findChunk(const uint8_t* data, size_t size, const char* chunkId, uint32_t& chunkSize);
};

} // namespace core
} // namespace j2me
