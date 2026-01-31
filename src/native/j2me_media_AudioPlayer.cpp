#include "j2me_media_AudioPlayer.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/SoundManager.hpp"
#include "../loader/JarLoader.hpp"
#include "../core/Logger.hpp"
#include "../native/java_lang_String.hpp"
#include "../core/Logger.hpp"
#include <cmath>
#include <sstream>
#include <unordered_map>
#include <string>
#include <mutex>

namespace j2me {
namespace natives {

// 存储 player 对象对应的资源名称（供其他文件访问）
std::unordered_map<j2me::core::JavaObject*, std::string> playerResources;
std::mutex playerResourcesMutex;

// 从 JAR 文件加载音频资源
bool loadAudioForPlayer(const std::string& locator, j2me::core::JavaObject* playerObj) {
    auto* jarLoader = j2me::core::NativeRegistry::getInstance().getJarLoader();
    if (!jarLoader) {
        LOG_ERROR("[AudioPlayer] No JAR loader available");
        return false;
    }

    // 解析 locator，获取资源路径
    // locator 格式通常是 "file:/bgm.mid" 或 "file:/audio/bgm.wav"
    std::string resourcePath;
    std::string audioType;

    if (locator.find("file:/") == 0) {
        resourcePath = locator.substr(6); // 跳过 "file:/"

        // 根据文件扩展名确定类型
        if (resourcePath.find(".mid") != std::string::npos ||
            resourcePath.find(".midi") != std::string::npos) {
            audioType = "audio/midi";
        } else if (resourcePath.find(".wav") != std::string::npos) {
            audioType = "audio/wav";
        } else {
            std::stringstream ss;
            ss << "[AudioPlayer] Unknown audio type for: " << resourcePath;
            LOG_ERROR(ss.str().c_str());
            return false;
        }
    } else {
        std::stringstream ss;
        ss << "[AudioPlayer] Invalid locator format: " << locator;
        LOG_ERROR(ss.str().c_str());
        return false;
    }

    // 从 JAR 读取音频文件数据
    auto data = jarLoader->getFile(resourcePath);
    if (!data) {
        std::stringstream ss;
        ss << "[AudioPlayer] Resource not found in JAR: " << resourcePath;
        LOG_ERROR(ss.str().c_str());
        return false;
    }

    // 根据类型加载到 SoundManager
    bool loaded = false;
    if (audioType == "audio/midi") {
        loaded = j2me::core::SoundManager::getInstance().loadMIDI(resourcePath, *data);
    } else if (audioType == "audio/wav") {
        loaded = j2me::core::SoundManager::getInstance().loadWAV(resourcePath, *data);
    }

    if (loaded) {
        // 存储 player 到资源的映射
        std::lock_guard<std::mutex> lock(playerResourcesMutex);
        playerResources[playerObj] = resourcePath;

        std::stringstream ss;
        ss << "[AudioPlayer] Loaded audio resource: " << resourcePath;
        LOG_INFO(ss.str().c_str());
        return true;
    }

    return false;
}

void registerAudioPlayerNatives(j2me::core::NativeRegistry& registry) {
    // j2me/media/AudioPlayer.nativeStart()V
    registry.registerNative("j2me/media/AudioPlayer", "nativeStart", "()V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            std::cout << "[AudioPlayer] nativeStart called" << std::endl;
            
            // 示例：播放一个简单的正弦波声音
            // 实际应用中，应该从音频文件或流中读取数据
            
            // 创建一个440Hz的正弦波
            std::vector<uint8_t> audioData;
            int sampleRate = 44100;
            int channels = 2;
            int sampleSize = 2; // 16位
            int duration = 1000; // 1秒
            
            double frequency = 440.0; // A4音
            double amplitude = 32767.0; // 最大振幅
            
            for (int i = 0; i < sampleRate * duration / 1000; i++) {
                double time = (double)i / sampleRate;
                double value = amplitude * sin(2 * M_PI * frequency * time);
                
                // 转换为16位PCM
                int16_t sample = (int16_t)value;
                
                // 左右声道
                for (int c = 0; c < channels; c++) {
                    audioData.push_back((uint8_t)(sample & 0xFF));
                    audioData.push_back((uint8_t)((sample >> 8) & 0xFF));
                }
            }
        }
    );

    // j2me/media/AudioPlayer.nativeStop()V
    registry.registerNative("j2me/media/AudioPlayer", "nativeStop", "()V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            std::cout << "[AudioPlayer] nativeStop called" << std::endl;
            j2me::core::AudioManager::getInstance().stopAudio();
        }
    );

    // j2me/media/AudioPlayer.nativeClose()V
    registry.registerNative("j2me/media/AudioPlayer", "nativeClose", "()V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            std::cout << "[AudioPlayer] nativeClose called" << std::endl;
            j2me::core::AudioManager::getInstance().stopAudio();
        }
    );
}

void registerAudioVolumeControlNatives(j2me::core::NativeRegistry& registry) {
    // j2me/media/AudioVolumeControl.nativeSetVolume(I)V
    registry.registerNative("j2me/media/AudioVolumeControl", "nativeSetVolume", "(I)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            auto volumeValue = frame->pop();
            int volume = volumeValue.val.i;
            
            std::cout << "[AudioVolumeControl] nativeSetVolume called with volume: " << volume << std::endl;
            
            // 设置音量
            j2me::core::AudioManager::getInstance().setVolume(volume);
        }
    );
}

} // namespace natives
} // namespace j2me
