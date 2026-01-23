#include "java_lang_Thread.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/EventLoop.hpp"
#include "../core/TimerManager.hpp"
#include <thread>
#include <chrono>
#include <iostream>

namespace j2me {
namespace natives {

void registerThreadNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();

    // java/lang/Thread.sleep(J)V
    registry.registerNative("java/lang/Thread", "sleep", "(J)V", 
        [&registry](std::shared_ptr<j2me::core::StackFrame> frame) {
            int64_t millis = frame->pop().val.l;
            
            if (millis < 0) {
                std::cerr << "IllegalArgumentException: timeout value is negative" << std::endl;
                // Should throw exception
                return;
            }
            
            auto interpreter = registry.getInterpreter();
            int64_t remaining = millis;
            
            // If millis is 0, just yield and process events once
            if (remaining == 0) {
                j2me::core::TimerManager::getInstance().tick(interpreter);
                j2me::core::EventLoop::runSingleStep(interpreter);
                std::this_thread::yield();
                return;
            }

            while (remaining > 0) {
                // Run event loop (Process Input & Render & Timer)
                j2me::core::TimerManager::getInstance().tick(interpreter);
                j2me::core::EventLoop::runSingleStep(interpreter);
                
                int64_t step = 16; // ~60 FPS
                if (step > remaining) step = remaining;
                
                std::this_thread::sleep_for(std::chrono::milliseconds(step));
                remaining -= step;
            }
        }
    );
}

} // namespace natives
} // namespace j2me
