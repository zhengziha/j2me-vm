#include "j2me_media_AudioPlayer.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/SoundManager.hpp"
#include "../loader/JarLoader.hpp"
#include "../core/Logger.hpp"
#include "../native/java_lang_String.hpp"
#include <iostream>
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
            // 获取当前对象（player 实例）- "this" 在栈顶
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* playerObj = nullptr;

            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                playerObj = reinterpret_cast<j2me::core::JavaObject*>(thisVal.val.ref);
            }

            if (!playerObj) {
                LOG_ERROR("[AudioPlayer] nativeStart: player object is null");
                return;
            }

            std::stringstream ss;
            ss << "[AudioPlayer] nativeStart called for player: " << playerObj;
            LOG_INFO(ss.str().c_str());

            // 初始化 SoundManager
            if (!j2me::core::SoundManager::getInstance().isInitialized()) {
                if (!j2me::core::SoundManager::getInstance().init()) {
                    LOG_ERROR("[AudioPlayer] Failed to initialize SoundManager");
                    return;
                }
            }

            // 查找这个 player 对应的资源
            std::lock_guard<std::mutex> lock(playerResourcesMutex);
            auto it = playerResources.find(playerObj);
            if (it != playerResources.end()) {
                const std::string& resourceName = it->second;

                // 播放 BGM（循环）
                bool success = j2me::core::SoundManager::getInstance().playBGM(resourceName, true);

                ss.str("");
                ss << "[AudioPlayer] " << (success ? "Started" : "Failed to start") << " playing: " << resourceName;
                LOG_INFO(ss.str().c_str());
            } else {
                LOG_ERROR("[AudioPlayer] No resource associated with this player");
            }
        }
    );

    // j2me/media/AudioPlayer.nativeStop()V
    registry.registerNative("j2me/media/AudioPlayer", "nativeStop", "()V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* playerObj = nullptr;

            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                playerObj = reinterpret_cast<j2me::core::JavaObject*>(thisVal.val.ref);
            }

            std::stringstream ss;
            ss << "[AudioPlayer] nativeStop called for player: " << playerObj;
            LOG_INFO(ss.str().c_str());

            // 停止 BGM
            j2me::core::SoundManager::getInstance().stopBGM();
        }
    );

    // j2me/media/AudioPlayer.nativeClose()V
    registry.registerNative("j2me/media/AudioPlayer", "nativeClose", "()V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* playerObj = nullptr;

            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                playerObj = reinterpret_cast<j2me::core::JavaObject*>(thisVal.val.ref);
            }

            std::stringstream ss;
            ss << "[AudioPlayer] nativeClose called for player: " << playerObj;
            LOG_INFO(ss.str().c_str());

            // 停止 BGM
            j2me::core::SoundManager::getInstance().stopBGM();

            // 清理资源映射
            if (playerObj) {
                std::lock_guard<std::mutex> lock(playerResourcesMutex);
                playerResources.erase(playerObj);
            }
        }
    );
}

void registerAudioVolumeControlNatives(j2me::core::NativeRegistry& registry) {
    // j2me/media/AudioVolumeControl.nativeSetVolume(I)V
    registry.registerNative("j2me/media/AudioVolumeControl", "nativeSetVolume", "(I)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            auto volumeValue = frame->pop();
            int volume = volumeValue.val.i;

            std::stringstream ss;
            ss << "[AudioVolumeControl] nativeSetVolume called with volume: " << volume;
            LOG_INFO(ss.str().c_str());

            j2me::core::SoundManager::getInstance().setBGMVolume(volume);
        }
    );

    // j2me/media/AudioVolumeControl.nativeGetVolume()I
    registry.registerNative("j2me/media/AudioVolumeControl", "nativeGetVolume", "()I",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int volume = j2me::core::SoundManager::getInstance().getBGMVolume();

            std::stringstream ss;
            ss << "[AudioVolumeControl] nativeGetVolume returning: " << volume;
            LOG_INFO(ss.str().c_str());

            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = volume;
            frame->push(result);
        }
    );
}

} // namespace natives
} // namespace j2me
