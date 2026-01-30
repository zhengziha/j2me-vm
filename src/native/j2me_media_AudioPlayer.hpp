#pragma once

#include "../core/NativeRegistry.hpp"

namespace j2me {
namespace core {
class JavaObject;
}

namespace natives {

// 注册AudioPlayer的本地方法
void registerAudioPlayerNatives(j2me::core::NativeRegistry& registry);

// 注册AudioVolumeControl的本地方法
void registerAudioVolumeControlNatives(j2me::core::NativeRegistry& registry);

// 为 player 对象加载音频资源（从 JAR 文件）
// 返回 true 表示加载成功，false 表示失败
bool loadAudioForPlayer(const std::string& locator, j2me::core::JavaObject* playerObj);

} // namespace natives
} // namespace j2me
