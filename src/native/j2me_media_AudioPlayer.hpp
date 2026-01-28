#pragma once

#include "../core/NativeRegistry.hpp"

namespace j2me {
namespace natives {

// 注册AudioPlayer的本地方法
void registerAudioPlayerNatives(j2me::core::NativeRegistry& registry);

// 注册AudioVolumeControl的本地方法
void registerAudioVolumeControlNatives(j2me::core::NativeRegistry& registry);

} // namespace natives
} // namespace j2me
