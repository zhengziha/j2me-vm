#include "javax_microedition_media_Manager.hpp"
#include "j2me_media_AudioPlayer.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "../core/SoundManager.hpp"
#include "../loader/JarLoader.hpp"
#include "../core/Logger.hpp"
#include "../native/java_lang_String.hpp"
#include <sstream>
#include <unordered_map>
#include <string>
#include <mutex>
#include "../core/Logger.hpp"

namespace j2me {
namespace natives {

// 从 JAR 文件加载音频资源
static bool loadAudioFromJar(const std::string& resourcePath, const std::string& audioType) {
    auto* jarLoader = j2me::core::NativeRegistry::getInstance().getJarLoader();
    if (!jarLoader) {
        LOG_ERROR("[Media] No JAR loader available");
        return false;
    }

    // 从 JAR 读取音频文件数据
    auto data = jarLoader->getFile(resourcePath);
    if (!data) {
        std::stringstream ss;
        ss << "[Media] Resource not found in JAR: " << resourcePath;
        LOG_ERROR(ss.str().c_str());
        return false;
    }

    // 根据类型加载
    if (audioType == "audio/X-wav" || audioType == "audio/wav") {
        return j2me::core::SoundManager::getInstance().loadWAV(resourcePath, *data);
    } else if (audioType == "audio/midi" || audioType == "audio/x-midi" ||
               audioType == "audio/X-midi" || audioType == "audio/mid") {
        return j2me::core::SoundManager::getInstance().loadMIDI(resourcePath, *data);
    }

    std::stringstream ss;
    ss << "[Media] Unsupported audio type: " << audioType;
    LOG_ERROR(ss.str().c_str());
    return false;
}

// 生成单音（TONE）
static bool generateTone(int note, int duration, int volume, std::vector<uint8_t>& audioData) {
    // MIDI 音符频率
    static const int noteFrequencies[] = {
        0,      // 0 - 无效
        65,     // 1 - C2
        73,     // 2 - C#2
        82,     // 3 - D2
        87,     // 4 - D#2
        98,     // 5 - E2
        110,    // 6 - F2
        123,    // 7 - F#2
        131,    // 8 - G2
        147,    // 9 - G#2
        165,    // 10 - A2
        175,    // 11 - A#2
        196,    // 12 - B2
        220,    // 13 - C3
        233,    // 14 - C#3
        247,    // 15 - D3
        262,    // 16 - D#3
        294,    // 17 - E3
        311,    // 18 - F3
        330,    // 19 - F#3
        349,    // 20 - G3
        370,    // 21 - G#3
        392,    // 22 - A3
        415,    // 23 - A#3
        440,    // 24 - B3
        494,    // 25 - C4
        523,    // 26 - C#4
        554,    // 27 - D4
        587,    // 28 - D#4
        622,    // 29 - E4
        659,    // 30 - F4
        698,    // 31 - F#4
        740,    // 32 - G4
        784,    // 33 - G#4
        831,    // 34 - A4
        880,    // 35 - A#4
        932,    // 36 - B4
        988,    // 37 - C5
        1047,   // 38 - C#5
        1109,   // 39 - D5
        1175,   // 40 - D#5
        1245,   // 41 - E5
        1319,   // 42 - F5
        1397,   // 43 - F#5
        1480,   // 44 - G5
        1568,   // 45 - G#5
        1661,   // 46 - A5
        1760,   // 47 - A#5
        1865,   // 48 - B5
        1976,   // 49 - C6
        2093,   // 50 - C#6
        2217,   // 51 - D6
        2349,   // 52 - D#6
        2489,   // 53 - E6
        2637,   // 54 - F6
        2794,   // 55 - F#6
        2960,   // 56 - G6
        3136,   // 57 - G#6
        3322,   // 58 - A6
        3520,   // 59 - A#6
        3729,   // 60 - B6
        3951    // 61 - C7
    };

    int noteIndex = (note >> 16) & 0xFF;
    int durationMs = note & 0xFFFF;

    if (noteIndex < 1 || noteIndex > 61) {
        return false;
    }

    int frequency = noteFrequencies[noteIndex];
    int sampleRate = 44100;
    int channels = 1; // 单音使用单声道
    int bitsPerSample = 16;
    int totalSamples = (sampleRate * durationMs) / 1000;

    double amplitude = 16383.0; // 中等音量

    audioData.clear();
    audioData.reserve(totalSamples * 2); // 16-bit = 2 bytes per sample

    for (int i = 0; i < totalSamples; i++) {
        double time = (double)i / sampleRate;
        double value = amplitude * sin(2 * M_PI * frequency * time);

        // 添加简单的包络（淡入淡出）
        int attackSamples = sampleRate / 100; // 10ms attack
        int releaseSamples = sampleRate / 20;  // 50ms release
        if (i < attackSamples) {
            value *= (double)i / attackSamples;
        } else if (i > totalSamples - releaseSamples) {
            value *= (double)(totalSamples - i) / releaseSamples;
        }

        int16_t sample = (int16_t)value;
        audioData.push_back((uint8_t)(sample & 0xFF));
        audioData.push_back((uint8_t)((sample >> 8) & 0xFF));
    }

    return true;
}

// 存储 player 对象对应的资源名称（与 j2me_media_AudioPlayer.cpp 共享）
extern std::unordered_map<j2me::core::JavaObject*, std::string> playerResources;
extern std::mutex playerResourcesMutex;

void registerMediaNatives(j2me::core::NativeRegistry& registry) {
    // javax/microedition/media/Manager.createPlayer(Ljava/lang/String;)Ljavax/microedition/media/Player;
    registry.registerNative("javax/microedition/media/Manager", "createPlayer", "(Ljava/lang/String;)Ljavax/microedition/media/Player;",
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            auto locatorValue = frame->pop();
            std::string locator = getJavaString(reinterpret_cast<j2me::core::JavaObject*>(locatorValue.val.ref));

            std::stringstream ss;
            ss << "[Media] createPlayer called with locator: " << locator;
            LOG_INFO(ss.str().c_str());

            // 初始化 SoundManager
            if (!j2me::core::SoundManager::getInstance().isInitialized()) {
                if (!j2me::core::SoundManager::getInstance().init()) {
                    LOG_ERROR("[Media] Failed to initialize SoundManager");
                    j2me::core::JavaValue result;
                    result.type = j2me::core::JavaValue::REFERENCE;
                    result.val.ref = nullptr;
                    frame->push(result);
                    return;
                }
            }

            auto interpreter = registry.getInterpreter();
            if (!interpreter) {
                LOG_ERROR("[Media] No interpreter available");
                j2me::core::JavaValue result;
                result.type = j2me::core::JavaValue::REFERENCE;
                result.val.ref = nullptr;
                frame->push(result);
                return;
            }

            auto playerCls = interpreter->resolveClass("j2me/media/AudioPlayer");
            if (!playerCls) {
                LOG_ERROR("[Media] Failed to resolve AudioPlayer class");
                j2me::core::JavaValue result;
                result.type = j2me::core::JavaValue::REFERENCE;
                result.val.ref = nullptr;
                frame->push(result);
                return;
            }

            interpreter->initializeClass(thread, playerCls);
            auto playerObj = j2me::core::HeapManager::getInstance().allocate(playerCls);

            // 加载音频资源并关联到 player 对象
            if (!loadAudioForPlayer(locator, playerObj)) {
                std::stringstream ss2;
                ss2 << "[Media] Failed to load audio for locator: " << locator;
                LOG_ERROR(ss2.str().c_str());
                // 继续执行，因为 player 对象已经创建
            }

            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = playerObj;
            frame->push(result);

            ss.str("");
            ss << "[Media] Created AudioPlayer instance: " << playerObj << " for locator: " << locator;
            LOG_INFO(ss.str().c_str());
        }
    );

    // javax/microedition/media/Manager.createPlayer(Ljava/io/InputStream;Ljava/lang/String;)Ljavax/microedition/media/Player;
    registry.registerNative("javax/microedition/media/Manager", "createPlayer", "(Ljava/io/InputStream;Ljava/lang/String;)Ljavax/microedition/media/Player;",
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            auto typeValue = frame->pop();
            auto streamValue = frame->pop();

            std::string type = getJavaString(reinterpret_cast<j2me::core::JavaObject*>(typeValue.val.ref));
            j2me::core::JavaObject* streamObj = reinterpret_cast<j2me::core::JavaObject*>(streamValue.val.ref);

            std::stringstream ss;
            ss << "[Media] createPlayer(InputStream, type) called with type: " << type;
            LOG_INFO(ss.str().c_str());

            // 初始化 SoundManager
            if (!j2me::core::SoundManager::getInstance().isInitialized()) {
                j2me::core::SoundManager::getInstance().init();
            }

            // 从 InputStream 读取音频数据
            std::vector<uint8_t> audioData;
            std::string resourcePath = "audio_from_stream";

            if (streamObj && streamObj->fields.size() > 0) {
                int streamId = streamObj->fields[0];
                auto* stream = j2me::core::HeapManager::getInstance().getStream(streamId);

                if (stream) {
                    // 获取流数据
                    const uint8_t* data = stream->getData();
                    size_t size = stream->getSize();

                    // 如果流有路径信息，使用它作为资源名
                    if (!stream->getFilePath().empty()) {
                        resourcePath = stream->getFilePath();
                    }

                    // 复制数据到 vector
                    audioData.assign(data, data + size);

                    ss.str("");
                    ss << "[Media] Read " << audioData.size() << " bytes from InputStream, path: " << resourcePath;
                    LOG_INFO(ss.str().c_str());
                } else {
                    ss.str("");
                    ss << "[Media] Failed to get stream with ID: " << streamId;
                    LOG_ERROR(ss.str().c_str());
                }
            }

            auto interpreter = registry.getInterpreter();
            if (!interpreter) {
                j2me::core::JavaValue result;
                result.type = j2me::core::JavaValue::REFERENCE;
                result.val.ref = nullptr;
                frame->push(result);
                return;
            }

            auto playerCls = interpreter->resolveClass("j2me/media/AudioPlayer");
            if (!playerCls) {
                j2me::core::JavaValue result;
                result.type = j2me::core::JavaValue::REFERENCE;
                result.val.ref = nullptr;
                frame->push(result);
                return;
            }

            interpreter->initializeClass(thread, playerCls);
            auto playerObj = j2me::core::HeapManager::getInstance().allocate(playerCls);

            // 加载音频数据到 SoundManager
            bool loaded = false;
            if (!audioData.empty()) {
                if (type == "audio/midi" || type == "audio/x-midi" ||
                    type == "audio/X-midi" || type == "audio/mid") {
                    loaded = j2me::core::SoundManager::getInstance().loadMIDI(resourcePath, audioData);
                } else if (type == "audio/X-wav" || type == "audio/wav") {
                    loaded = j2me::core::SoundManager::getInstance().loadWAV(resourcePath, audioData);
                } else {
                    // 根据文件扩展名猜测类型
                    if (resourcePath.find(".mid") != std::string::npos ||
                        resourcePath.find(".midi") != std::string::npos) {
                        loaded = j2me::core::SoundManager::getInstance().loadMIDI(resourcePath, audioData);
                    } else if (resourcePath.find(".wav") != std::string::npos) {
                        loaded = j2me::core::SoundManager::getInstance().loadWAV(resourcePath, audioData);
                    }
                }

                if (loaded) {
                    // 存储 player 到资源的映射
                    std::lock_guard<std::mutex> lock(playerResourcesMutex);
                    playerResources[playerObj] = resourcePath;

                    ss.str("");
                    ss << "[Media] Loaded audio from InputStream: " << resourcePath;
                    LOG_INFO(ss.str().c_str());
                } else {
                    ss.str("");
                    ss << "[Media] Failed to load audio from InputStream, type: " << type;
                    LOG_ERROR(ss.str().c_str());
                }
            }

            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = playerObj;
            frame->push(result);

            ss.str("");
            ss << "[Media] Created AudioPlayer from InputStream: " << playerObj;
            LOG_INFO(ss.str().c_str());
        }
    );

    // javax/microedition/media/Manager.playTone(IIZ)V
    registry.registerNative("javax/microedition/media/Manager", "playTone", "(IIZ)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            auto durationValue = frame->pop();
            auto noteValue = frame->pop();
            auto volumeValue = frame->pop();

            int note = noteValue.val.i;
            int duration = durationValue.val.i;

            std::stringstream ss;
            ss << "[Media] playTone called: note=" << note << ", duration=" << duration;
            LOG_INFO(ss.str().c_str());

            // 初始化 SoundManager
            if (!j2me::core::SoundManager::getInstance().isInitialized()) {
                j2me::core::SoundManager::getInstance().init();
            }

            // 生成单音数据
            std::vector<uint8_t> toneData;
            if (generateTone(note, duration, 100, toneData)) {
                // 创建临时 AudioResource
                j2me::core::AudioResource resource;
                resource.data = toneData;
                resource.sampleRate = 44100;
                resource.channels = 1;
                resource.bitsPerSample = 16;
                resource.name = "tone";

                // 播放为 SFX（不循环）
                // 直接将资源加载并播放
                j2me::core::SoundManager::getInstance().loadWAV("tone", toneData);
                j2me::core::SoundManager::getInstance().playSFX("tone", false);

                LOG_INFO("[Media] Playing tone");
            } else {
                LOG_ERROR("[Media] Failed to generate tone");
            }
        }
    );
}

} // namespace natives
} // namespace j2me
