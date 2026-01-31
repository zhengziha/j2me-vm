#include "j2me_media_AudioPlayer.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/AudioManager.hpp"
#include "../core/Logger.hpp"
#include <cmath>

namespace j2me {
namespace natives {

void registerAudioPlayerNatives(j2me::core::NativeRegistry& registry) {
    // 注册AudioPlayer的本地方法
    
    // j2me/media/AudioPlayer.nativeStart()V
    registry.registerNative("j2me/media/AudioPlayer", "nativeStart", "()V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            LOG_DEBUG("[AudioPlayer] nativeStart called");
            
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
            
            // 播放音频
            j2me::core::AudioManager::getInstance().playAudio(audioData, sampleRate, channels, sampleSize);
        }
    );
    
    // j2me/media/AudioPlayer.nativeStop()V
    registry.registerNative("j2me/media/AudioPlayer", "nativeStop", "()V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            LOG_DEBUG("[AudioPlayer] nativeStop called");
            j2me::core::AudioManager::getInstance().stopAudio();
        }
    );
    
    // j2me/media/AudioPlayer.nativeClose()V
    registry.registerNative("j2me/media/AudioPlayer", "nativeClose", "()V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            LOG_DEBUG("[AudioPlayer] nativeClose called");
            j2me::core::AudioManager::getInstance().stopAudio();
        }
    );
}

void registerAudioVolumeControlNatives(j2me::core::NativeRegistry& registry) {
    // j2me/media/AudioVolumeControl.nativeSetVolume(I)V
    registry.registerNative("j2me/media/AudioVolumeControl", "nativeSetVolume", "(I)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            // 获取音量参数
            auto volumeValue = frame->pop();
            int volume = volumeValue.val.i;
            
            LOG_DEBUG("[AudioVolumeControl] nativeSetVolume called with volume: " + std::to_string(volume));
            
            // 设置音量
            j2me::core::AudioManager::getInstance().setVolume(volume);
        }
    );
}

} // namespace natives
} // namespace j2me
