#include "java_util_Timer.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/TimerManager.hpp"
#include <iostream>

namespace j2me {
namespace natives {

void registerTimerNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();

    // java/util/Timer.scheduleNative(Ljava/util/TimerTask;JJ)V
    registry.registerNative("java/util/Timer", "scheduleNative", "(Ljava/util/TimerTask;JJ)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int64_t period = frame->pop().val.l;
            int64_t delay = frame->pop().val.l;
            j2me::core::JavaValue taskVal = frame->pop();
            frame->pop(); // this (Timer)
            
            j2me::core::JavaObject* task = (j2me::core::JavaObject*)taskVal.val.ref;
            
            if (task) {
                // std::cout << "[Timer] Scheduled task: " << task << " delay=" << delay << " period=" << period << std::endl;
                j2me::core::TimerManager::getInstance().schedule(task, delay, period);
            }
        }
    );
}

}
}
