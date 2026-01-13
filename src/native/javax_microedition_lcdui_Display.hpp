#pragma once
#include "../core/RuntimeTypes.hpp"
#include <memory>

namespace j2me {
namespace natives {

void registerDisplayNatives();

// Helper to access the current displayable from Main loop
j2me::core::JavaObject* getCurrentDisplayable();

}
}
