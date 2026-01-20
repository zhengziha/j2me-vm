#include "java_lang_Thread.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include <thread>
#include <chrono>
#include <iostream>

namespace j2me {
namespace natives {

void registerThreadNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();

    // java/lang/Thread.sleep(J)V
    registry.registerNative("java/lang/Thread", "sleep", "(J)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int64_t millis = frame->pop().val.l;
            
            if (millis < 0) {
                std::cerr << "IllegalArgumentException: timeout value is negative" << std::endl;
                // Should throw exception
                return;
            }
            
            // Sleep
            // std::cout << "Thread.sleep(" << millis << ")" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(millis));
        }
    );
}

} // namespace natives
} // namespace j2me
