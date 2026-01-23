#pragma once

#include "Interpreter.hpp"
#include "JavaThread.hpp"
#include <queue>
#include <mutex>
#include <atomic>
#include <memory>

namespace j2me {
namespace core {

struct KeyEvent {
    int keyCode;
    // type: pressed/released? For now just pressed
};

class EventLoop {
public:
    static EventLoop& getInstance() {
        static EventLoop instance;
        return instance;
    }

    // Called by Main Thread
    void pollSDL(); 
    
    // Called by VM Thread
    void dispatchEvents(Interpreter* interpreter);
    void render(Interpreter* interpreter);
    
    bool shouldExit() const { return quit; }
    int getKeyStates() const { return keyStates; }

    // Legacy static wrappers (deprecated/modified)
    static void processEvents(Interpreter* interpreter) {
        getInstance().dispatchEvents(interpreter);
    }
    static void renderStatic(Interpreter* interpreter) {
        getInstance().render(interpreter);
    }
    static void runSingleStep(Interpreter* interpreter) {
        getInstance().dispatchEvents(interpreter);
        getInstance().render(interpreter);
    }

private:
    std::queue<int> eventQueue; // Stores mapped keyCodes
    std::mutex queueMutex;
    std::atomic<bool> quit{false};
    std::atomic<int> keyStates{0};
    
    // Track current painting thread to avoid flooding
    std::weak_ptr<JavaThread> paintingThread;
};

} // namespace core
} // namespace j2me
