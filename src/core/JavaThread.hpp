#pragma once

#include "StackFrame.hpp"
#include <vector>
#include <memory>
#include <string>

namespace j2me {
namespace core {

class JavaThread {
public:
    enum State {
        NEW,
        RUNNABLE,
        BLOCKED,
        WAITING,
        TIMED_WAITING,
        TERMINATED
    };

    JavaThread(std::shared_ptr<StackFrame> initialFrame) 
        : state(RUNNABLE), wakeTime(0) {
        frames.push_back(initialFrame);
    }

    void pushFrame(std::shared_ptr<StackFrame> frame) {
        frames.push_back(frame);
    }

    void popFrame() {
        if (!frames.empty()) {
            frames.pop_back();
        }
    }

    std::shared_ptr<StackFrame> currentFrame() {
        if (frames.empty()) return nullptr;
        return frames.back();
    }

    bool isFinished() const {
        return frames.empty();
    }

    State state;
    long long wakeTime; // For sleep/wait
    void* waitingOn = nullptr; // Object address being waited on
    void* javaThreadObject = nullptr; // Associated java.lang.Thread object address
    std::vector<std::shared_ptr<StackFrame>> frames;
    
    // Associated Java Thread Object (optional for now, but good for future)
    // JavaObject* javaThreadObj = nullptr; 
};

} // namespace core
} // namespace j2me
