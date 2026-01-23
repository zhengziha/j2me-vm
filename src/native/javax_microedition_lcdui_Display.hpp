#pragma once
#include "../core/RuntimeTypes.hpp"
#include <memory>

namespace j2me {
namespace core {
    class NativeRegistry;
}
namespace natives {

void registerDisplayNatives(j2me::core::NativeRegistry& registry);

// Helper to access the current displayable from Main loop
j2me::core::JavaObject* getCurrentDisplayable();

}
}
