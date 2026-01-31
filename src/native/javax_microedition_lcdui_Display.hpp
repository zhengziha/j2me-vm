#pragma once
#include "../core/RuntimeTypes.hpp"
#include "../core/Interpreter.hpp"
#include "../core/HeapManager.hpp"
#include <memory>

namespace j2me {
namespace core {
    class NativeRegistry;
}
namespace natives {

void registerDisplayNatives(j2me::core::NativeRegistry& registry);

// Helper to access the current displayable from Main loop
j2me::core::JavaObject* getCurrentDisplayable();

// J2ME 规范的 repaint 请求管理
// Repaint request management according to J2ME specification
struct RepaintRequest {
    bool pending = false;
    int x = 0, y = 0, width = 0, height = 0;
    bool fullScreen = true;

    void request(int x_, int y_, int w_, int h_) {
        pending = true;
        x = x_; y = y_; width = w_; height = h_;
        fullScreen = false;
    }

    void requestFull() {
        pending = true;
        fullScreen = true;
    }

    void clear() {
        pending = false;
        fullScreen = true;
    }
};

// 获取当前的重绘请求
RepaintRequest& getRepaintRequest();

// 立即执行挂起的重绘 (serviceRepaints)
void serviceRepaints(j2me::core::Interpreter* interpreter);

}
}
