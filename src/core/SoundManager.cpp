#include "SoundManager.hpp"
#include "MIDIPlayer.hpp"
#include "Logger.hpp"
#include <cstring>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace j2me {
namespace core {

// ==================== BGMPlayer ====================

BGMPlayer::BGMPlayer()
    : position(0), playing(false), paused(false), volume(100), loop(false), isMIDI(false)
{
}

BGMPlayer::~BGMPlayer() {
    stop();
}

bool BGMPlayer::load(const std::string& name, const AudioResource& resource) {
    std::lock_guard<std::mutex> lock(mutex);

    // 清理之前的播放器
    midiPlayer.reset();
    currentBGM.data.clear();

    if (resource.type == AUDIO_MIDI) {
        // 加载 MIDI
        midiPlayer = std::make_unique<MIDIPlayer>();
        if (!midiPlayer->loadFromMemory(resource.data)) {
            std::stringstream ss;
            ss << "Failed to load MIDI: " << name;
            LOG_ERROR(ss.str().c_str());
            return false;
        }
        currentBGM = resource;
        currentBGM.name = name;
        isMIDI = true;
        position = 0;

        std::stringstream ss;
        ss << "Loaded MIDI BGM: " << name << " (" << midiPlayer->getDurationMs() << " ms)";
        LOG_INFO(ss.str().c_str());
    } else {
        // 加载 WAV
        currentBGM = resource;
        currentBGM.name = name;
        isMIDI = false;
        position = 0;

        std::stringstream ss;
        ss << "Loaded WAV BGM: " << name << " (" << resource.data.size() << " bytes)";
        LOG_INFO(ss.str().c_str());
    }

    return true;
}

void BGMPlayer::play() {
    std::lock_guard<std::mutex> lock(mutex);

    if (isMIDI && midiPlayer) {
        midiPlayer->setLoop(loop);
        midiPlayer->play();
        playing = true;
        paused = false;
    } else if (!currentBGM.data.empty()) {
        position = 0;
        playing = true;
        paused = false;
    }
}

void BGMPlayer::stop() {
    std::lock_guard<std::mutex> lock(mutex);

    if (isMIDI && midiPlayer) {
        midiPlayer->stop();
        playing = false;
    }

    playing = false;
    paused = false;
    position = 0;
}

void BGMPlayer::pause() {
    std::lock_guard<std::mutex> lock(mutex);

    if (isMIDI && midiPlayer) {
        midiPlayer->pause();
    }

    paused = true;
}

void BGMPlayer::resume() {
    std::lock_guard<std::mutex> lock(mutex);

    if (isMIDI && midiPlayer) {
        midiPlayer->resume();
    }

    paused = false;
}

bool BGMPlayer::isPlaying() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex));

    if (isMIDI && midiPlayer) {
        return midiPlayer->isPlaying();
    }

    return playing && !paused;
}

void BGMPlayer::setVolume(int vol) {
    std::lock_guard<std::mutex> lock(mutex);
    volume = std::max(0, std::min(100, vol));

    if (isMIDI && midiPlayer) {
        midiPlayer->setVolume(vol);
    }
}

int BGMPlayer::getVolume() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex));
    return volume;
}

void BGMPlayer::setLoop(bool l) {
    std::lock_guard<std::mutex> lock(mutex);
    loop = l;

    if (isMIDI && midiPlayer) {
        midiPlayer->setLoop(l);
    }
}

bool BGMPlayer::getAudioData(uint8_t* stream, int len) {
    std::lock_guard<std::mutex> lock(mutex);

    // 调试输出已禁用

    if (!playing || paused) {
        return false;
    }

    if (isMIDI && midiPlayer) {
        // MIDI 播放
        if (midiPlayer->isPlaying()) {
            // MIDI 合成器生成单声道样本，需要转换为立体声
            // len = 16384 字节 = 8192 int16_t (立体声)
            // 我们需要生成 4096 个单声道样本，然后复制到左右声道
            int monoSamples = (len / 2) / 2;  // 单声道样本数 = 立体声样本数 / 2
            std::vector<int16_t> tempBuffer(monoSamples);
            bool hasData = midiPlayer->getAudioData(tempBuffer.data(), monoSamples);

            if (hasData) {

                // 复制到输出（单声道 -> 立体声）
                int16_t* src = tempBuffer.data();
                int16_t* dst = reinterpret_cast<int16_t*>(stream);
                for (int i = 0; i < monoSamples; i++) {
                    int16_t sample = *src++;
                    *dst++ = sample;  // 左声道
                    *dst++ = sample;  // 右声道
                }
                return true;
            }
        }
        return false;
    } else {
        // WAV 播放
        if (currentBGM.data.empty()) {
            return false;
        }

        int bytesPerSample = currentBGM.bitsPerSample / 8;
        int samplesToRead = len / bytesPerSample;
        int bytesAvailable = currentBGM.data.size() - position;
        int bytesToCopy = std::min(len, bytesAvailable);

        if (bytesToCopy > 0) {
            // 检查数据边界
            if (position + bytesToCopy > currentBGM.data.size()) {
                bytesToCopy = currentBGM.data.size() - position;
            }
            
            std::memcpy(stream, currentBGM.data.data() + position, bytesToCopy);
            position += bytesToCopy;

            // 检查是否播放完毕
            if (position >= currentBGM.data.size()) {
                if (loop) {
                    position = 0; // 循环播放
                } else {
                    playing = false;
                    position = 0;
                }
            }

            return true;
        }
    }

    return false;
}

// ==================== SFXPlayer ====================

SFXPlayer::SFXPlayer() {
    channels.reserve(MAX_CHANNELS);
    for (int i = 0; i < MAX_CHANNELS; i++) {
        channels.push_back(AudioChannel());
    }
}

SFXPlayer::~SFXPlayer() {
    stopAll();
}

int SFXPlayer::play(const AudioResource& resource, bool loop) {
    std::lock_guard<std::mutex> lock(mutex);

    // 查找空闲通道
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (!channels[i].active) {
            channels[i].data = resource.data;
            channels[i].position = 0;
            channels[i].active = true;
            channels[i].volume = 100;
            channels[i].loop = loop;
            return i;
        }
    }

    return -1; // 没有空闲通道
}

void SFXPlayer::stop(int channelId) {
    if (channelId < 0 || channelId >= MAX_CHANNELS) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex);
    channels[channelId].active = false;
    channels[channelId].data.clear();
    channels[channelId].position = 0;
}

void SFXPlayer::stopAll() {
    std::lock_guard<std::mutex> lock(mutex);
    for (auto& channel : channels) {
        channel.active = false;
        channel.data.clear();
        channel.position = 0;
    }
}

void SFXPlayer::setVolume(int channelId, int vol) {
    if (channelId < 0 || channelId >= MAX_CHANNELS) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex);
    channels[channelId].volume = std::max(0, std::min(100, vol));
}

bool SFXPlayer::isActive(int channelId) const {
    if (channelId < 0 || channelId >= MAX_CHANNELS) {
        return false;
    }

    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex));
    return channels[channelId].active;
}

void SFXPlayer::mixAudio(uint8_t* stream, int len) {
    std::lock_guard<std::mutex> lock(mutex);

    // 首先清空输出缓冲区
    std::memset(stream, 0, len);

    // 混合所有活跃通道
    for (auto& channel : channels) {
        if (!channel.active || channel.data.empty()) {
            continue;
        }

        int bytesPerSample = 2; // 16-bit
        int samplesToMix = len / bytesPerSample;
        int16_t* output = reinterpret_cast<int16_t*>(stream);

        for (int i = 0; i < samplesToMix; i++) {
            if (channel.position >= channel.data.size()) {
                if (channel.loop) {
                    channel.position = 0;
                } else {
                    channel.active = false;
                    break;
                }
            }

            // 从通道读取样本
            int16_t sample = *reinterpret_cast<int16_t*>(channel.data.data() + channel.position);

            // 应用通道音量
            sample = (sample * channel.volume) / 100;

            // 混合到输出（带防止溢出的裁剪）
            int32_t mixed = static_cast<int32_t>(output[i]) + sample;
            if (mixed > 32767) mixed = 32767;
            if (mixed < -32768) mixed = -32768;
            output[i] = static_cast<int16_t>(mixed);

            channel.position += bytesPerSample;
        }
    }
}

// ==================== SoundManager ====================

std::unique_ptr<SoundManager> SoundManager::instance = nullptr;
std::mutex SoundManager::instanceMutex;

SoundManager::SoundManager()
    : audioDevice(0)
    , initialized(false)
    , masterVolume(100)
{
}

SoundManager::~SoundManager() {
    shutdown();
}

SoundManager& SoundManager::getInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (!instance) {
        instance = std::unique_ptr<SoundManager>(new SoundManager());
    }
    return *instance;
}

bool SoundManager::init() {
    std::lock_guard<std::mutex> lock(audioMutex);

    if (initialized) {
        return true;
    }

    // 初始化 SDL 音频子系统
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        std::stringstream ss;
        ss << "Failed to initialize SDL audio: " << SDL_GetError();
        LOG_ERROR(ss.str().c_str());
        return false;
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
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return false;
    }

    // 启动音频设备
    SDL_PauseAudioDevice(audioDevice, 0);

    initialized = true;
    LOG_INFO("SoundManager initialized successfully");
    return true;
}

void SoundManager::shutdown() {
    std::lock_guard<std::mutex> lock(audioMutex);

    if (!initialized) {
        return;
    }

    if (audioDevice != 0) {
        SDL_PauseAudioDevice(audioDevice, 1);
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
    }

    // 清理 SDL 音频子系统
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    clearCache();

    initialized = false;
    LOG_INFO("SoundManager shutdown");
}

bool SoundManager::isInitialized() const {
    return initialized;
}

void SoundManager::audioCallback(void* userdata, uint8_t* stream, int len) {
    SoundManager* manager = static_cast<SoundManager*>(userdata);

    // 清空输出缓冲区
    std::memset(stream, 0, len);

    // 调试输出已禁用

    // 混合 BGM
    if (manager->bgmPlayer.getAudioData(stream, len)) {
        manager->applyVolume(stream, len, manager->bgmPlayer.getVolume());
    }

    // 混合 SFX
    manager->sfxPlayer.mixAudio(stream, len);

    // 应用主音量
    manager->applyVolume(stream, len, manager->masterVolume);
}

void SoundManager::applyVolume(uint8_t* stream, int len, int volume) {
    if (volume >= 100) {
        return;
    }

    float volumeFactor = static_cast<float>(volume) / 100.0f;
    int16_t* samples = reinterpret_cast<int16_t*>(stream);
    int sampleCount = len / sizeof(int16_t);

    for (int i = 0; i < sampleCount; i++) {
        samples[i] = static_cast<int16_t>(samples[i] * volumeFactor);
    }
}

// ==================== 资源管理 ====================

bool SoundManager::loadWAV(const std::string& name, const std::vector<uint8_t>& wavData) {
    AudioResource resource;
    if (!WAVLoader::parse(wavData, resource)) {
        std::stringstream ss;
        ss << "Failed to load WAV: " << name;
        LOG_ERROR(ss.str().c_str());
        return false;
    }

    resource.type = AUDIO_WAV;
    resource.name = name;

    std::lock_guard<std::mutex> lock(resourceMutex);
    resourceCache[name] = resource;

    std::stringstream ss;
    ss << "Loaded WAV: " << name << " (" << resource.data.size() << " bytes, "
       << resource.sampleRate << " Hz, " << resource.channels << " channels)";
    LOG_INFO(ss.str().c_str());
    return true;
}

bool SoundManager::loadMIDI(const std::string& name, const std::vector<uint8_t>& midData) {
    AudioResource resource;
    resource.data = midData;
    resource.type = AUDIO_MIDI;
    resource.name = name;
    resource.sampleRate = 44100;
    resource.channels = 2;
    resource.bitsPerSample = 16;

    std::lock_guard<std::mutex> lock(resourceMutex);
    resourceCache[name] = resource;

    std::stringstream ss;
    ss << "Loaded MIDI: " << name << " (" << midData.size() << " bytes)";
    LOG_INFO(ss.str().c_str());
    return true;
}

bool SoundManager::loadWAVFromFile(const std::string& name, const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::stringstream ss;
        ss << "Failed to open file: " << filePath;
        LOG_ERROR(ss.str().c_str());
        return false;
    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    return loadWAV(name, data);
}

const AudioResource* SoundManager::getResource(const std::string& name) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(resourceMutex));
    auto it = resourceCache.find(name);
    if (it != resourceCache.end()) {
        return &it->second;
    }
    return nullptr;
}

void SoundManager::unloadResource(const std::string& name) {
    std::lock_guard<std::mutex> lock(resourceMutex);
    resourceCache.erase(name);
}

void SoundManager::clearCache() {
    std::lock_guard<std::mutex> lock(resourceMutex);
    resourceCache.clear();
}

// ==================== BGM 控制 ====================

bool SoundManager::playBGM(const std::string& name, bool loop) {
    const AudioResource* resource = getResource(name);
    if (!resource) {
        std::stringstream ss;
        ss << "BGM resource not found: " << name;
        LOG_ERROR(ss.str().c_str());
        return false;
    }

    bgmPlayer.load(name, *resource);
    bgmPlayer.setLoop(loop);
    bgmPlayer.play();

    std::stringstream ss;
    ss << "Playing BGM: " << name << " (loop=" << (loop ? "yes" : "no") << ")";
    LOG_INFO(ss.str().c_str());
    return true;
}

void SoundManager::stopBGM() {
    bgmPlayer.stop();
}

void SoundManager::pauseBGM() {
    bgmPlayer.pause();
}

void SoundManager::resumeBGM() {
    bgmPlayer.resume();
}

bool SoundManager::isBGMPlaying() const {
    return bgmPlayer.isPlaying();
}

void SoundManager::setBGMVolume(int volume) {
    bgmPlayer.setVolume(volume);
}

int SoundManager::getBGMVolume() const {
    return bgmPlayer.getVolume();
}

// ==================== SFX 控制 ====================

int SoundManager::playSFX(const std::string& name, bool loop) {
    const AudioResource* resource = getResource(name);
    if (!resource) {
        std::stringstream ss;
        ss << "SFX resource not found: " << name;
        LOG_ERROR(ss.str().c_str());
        return -1;
    }

    int channelId = sfxPlayer.play(*resource, loop);
    if (channelId >= 0) {
        std::stringstream ss;
        ss << "Playing SFX: " << name << " (channel=" << channelId << ")";
        LOG_INFO(ss.str().c_str());
    }
    return channelId;
}

void SoundManager::stopSFX(int channelId) {
    sfxPlayer.stop(channelId);
}

void SoundManager::stopAllSFX() {
    sfxPlayer.stopAll();
}

void SoundManager::setSFXVolume(int channelId, int volume) {
    sfxPlayer.setVolume(channelId, volume);
}

// ==================== 全局音量 ====================

void SoundManager::setMasterVolume(int volume) {
    std::lock_guard<std::mutex> lock(audioMutex);
    masterVolume = std::max(0, std::min(100, volume));
}

int SoundManager::getMasterVolume() const {
    return masterVolume;
}

// ==================== WAVLoader ====================

const uint8_t* WAVLoader::findChunk(const uint8_t* data, size_t size, const char* chunkId, uint32_t& chunkSize) {
    const uint8_t* p = data + 12; // 跳过 RIFF 头

    while (p < data + size - 8) {
        // 读取 chunk ID 和大小
        if (std::memcmp(p, chunkId, 4) == 0) {
            chunkSize = *reinterpret_cast<const uint32_t*>(p + 4);
            return p + 8; // 返回 chunk 数据起始位置
        }

        // 跳到下一个 chunk
        uint32_t cs = *reinterpret_cast<const uint32_t*>(p + 4);
        p += 8 + cs;
    }

    return nullptr;
}

bool WAVLoader::parse(const std::vector<uint8_t>& wavData, AudioResource& resource) {
    if (wavData.size() < sizeof(WAVHeader)) {
        LOG_ERROR("WAV file too small: missing header");
        return false;
    }

    const WAVHeader* header = reinterpret_cast<const WAVHeader*>(wavData.data());

    // 验证 RIFF 和 WAVE 标识
    if (std::memcmp(header->riff, "RIFF", 4) != 0 ||
        std::memcmp(header->wave, "WAVE", 4) != 0) {
        LOG_ERROR("Invalid WAV file: missing RIFF or WAVE identifier");
        return false;
    }

    // 只支持 PCM 格式
    if (header->audioFormat != 1) {
        LOG_ERROR("Unsupported WAV format (only PCM is supported)");
        return false;
    }

    resource.sampleRate = header->sampleRate;
    resource.channels = header->numChannels;
    resource.bitsPerSample = header->bitsPerSample;

    // 查找 data chunk
    uint32_t dataSize = 0;
    const uint8_t* dataStart = findChunk(wavData.data(), wavData.size(), "data", dataSize);

    if (!dataStart) {
        LOG_ERROR("No data chunk found in WAV file");
        return false;
    }

    // 检查数据边界
    size_t dataEnd = (dataStart - wavData.data()) + dataSize;
    if (dataEnd > wavData.size()) {
        LOG_ERROR("WAV data chunk exceeds file size");
        return false;
    }

    // 复制音频数据
    resource.data.assign(dataStart, dataStart + dataSize);

    std::stringstream ss;
    ss << "WAV parsed: " << resource.sampleRate << "Hz, "
       << resource.channels << "ch, " << resource.bitsPerSample << "bit, "
       << resource.data.size() << " bytes";
    LOG_INFO(ss.str().c_str());

    return true;
}

bool WAVLoader::loadFromFile(const std::string& filePath, AudioResource& resource) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return false;
    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    return parse(data, resource);
}

} // namespace core
} // namespace j2me
