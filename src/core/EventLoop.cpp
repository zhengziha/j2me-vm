#include "EventLoop.hpp"
#include "NativeRegistry.hpp"
#include "HeapManager.hpp"
#include "../native/javax_microedition_lcdui_Display.hpp"
#include "../platform/GraphicsContext.hpp"
#include "ThreadManager.hpp"
#include <SDL2/SDL.h>
#include <iostream>
#include <thread>
#include <chrono>

namespace j2me {
namespace core {

static int mapKey(SDL_Keycode key) {
    switch (key) {
        case SDLK_UP: return 1; // UP
        case SDLK_DOWN: return 6; // DOWN
        case SDLK_LEFT: return 2; // LEFT
        case SDLK_RIGHT: return 5; // RIGHT
        case SDLK_RETURN: return 8; // FIRE
        case SDLK_0: return 48;
        case SDLK_1: return 49;
        case SDLK_2: return 50;
        case SDLK_3: return 51;
        case SDLK_4: return 52;
        case SDLK_5: return 53;
        case SDLK_6: return 54;
        case SDLK_7: return 55;
        case SDLK_8: return 56;
        case SDLK_9: return 57;
        case SDLK_F1: return -6; // Soft 1 (Left Soft)
        case SDLK_F2: return -7; // Soft 2 (Right Soft)
        // case SDLK_8: return 42; // STAR
        default: return 0;
    }
}

void EventLoop::pollSDL() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else if (e.type == SDL_KEYDOWN) {
            int keyCode = mapKey(e.key.keysym.sym);
            if (keyCode != 0) {
                std::lock_guard<std::mutex> lock(queueMutex);
                eventQueue.push(keyCode);
                
                // Update keyStates (GameCanvas)
                // Need to map keyCode to GameAction bit
                // UP=1 -> bit 1 (1<<1 = 2)
                // DOWN=6 -> bit 6 (1<<6 = 64)
                // LEFT=2 -> bit 2 (4)
                // RIGHT=5 -> bit 5 (32)
                // FIRE=8 -> bit 8 (256)
                // GAME_A=9 -> bit 9 (512)
                // GAME_B=10 -> bit 10 (1024)
                // GAME_C=11 -> bit 11 (2048)
                // GAME_D=12 -> bit 12 (4096)
                if (keyCode > 0 && keyCode <= 31) { // Safety check
                    keyStates |= (1 << keyCode);
                }
            }
        } else if (e.type == SDL_KEYUP) {
            int keyCode = mapKey(e.key.keysym.sym);
            if (keyCode != 0) {
                if (keyCode > 0 && keyCode <= 31) {
                    keyStates &= ~(1 << keyCode);
                }
            }
        }
    }
    
    // Refresh screen (Main Thread)
    // This ensures window surface update happens on the main thread
    j2me::platform::GraphicsContext::getInstance().update();
}

void EventLoop::dispatchEvents(Interpreter* interpreter) {
    std::queue<int> events;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (eventQueue.empty()) return;
        std::swap(events, eventQueue);
    }
    
    while (!events.empty()) {
        int keyCode = events.front();
        events.pop();
        
        j2me::core::JavaObject* displayable = j2me::natives::getCurrentDisplayable();
        if (displayable && displayable->cls) {
            // Call keyPressed(keyCode)
            // Walk up class hierarchy to find method
            auto currentCls = displayable->cls;
            while (currentCls) {
                bool found = false;
                for (const auto& method : currentCls->rawFile->methods) {
                    auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(currentCls->rawFile->constant_pool[method.name_index]);
                    if (name->bytes == "keyPressed") {
                        auto frame = std::make_shared<j2me::core::StackFrame>(method, currentCls->rawFile);
                        
                        // Push 'this'
                        j2me::core::JavaValue vThis; vThis.type = j2me::core::JavaValue::REFERENCE; vThis.val.ref = displayable;
                        frame->setLocal(0, vThis);
                        
                        // Push keyCode
                        j2me::core::JavaValue vKey; vKey.type = j2me::core::JavaValue::INT; vKey.val.i = keyCode;
                        frame->setLocal(1, vKey);
                        
                        auto thread = std::make_shared<JavaThread>(frame);
                        ThreadManager::getInstance().addThread(thread);
                        found = true;
                        break;
                    }
                }
                if (found) break;
                currentCls = currentCls->superClass;
            }
        }
    }
}

void EventLoop::render(Interpreter* interpreter) {
    j2me::core::JavaObject* displayable = j2me::natives::getCurrentDisplayable();
    
    if (displayable) {
        static std::shared_ptr<j2me::core::JavaClass> graphicsCls;
        if (!graphicsCls) {
            graphicsCls = interpreter->resolveClass("javax/microedition/lcdui/Graphics");
        }
        
        if (graphicsCls) {
            // Allocate Graphics object
            j2me::core::JavaObject* g = j2me::core::HeapManager::getInstance().allocate(graphicsCls);
            
            // Find paint method
            auto currentCls = displayable->cls;
            while (currentCls) {
                bool found = false;
                for (const auto& method : currentCls->rawFile->methods) {
                    auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(currentCls->rawFile->constant_pool[method.name_index]);
                    if (name->bytes == "paint") {
                        static int paintCount = 0;
                        if (paintCount++ % 60 == 0) std::cout << "[EventLoop] Calling paint()" << std::endl;

                        auto frame = std::make_shared<j2me::core::StackFrame>(method, currentCls->rawFile);
                        
                        // Push 'this' (Canvas)
                        j2me::core::JavaValue vThis; vThis.type = j2me::core::JavaValue::REFERENCE; vThis.val.ref = displayable;
                        frame->setLocal(0, vThis);
                        
                        // Push 'Graphics'
                        j2me::core::JavaValue vG; vG.type = j2me::core::JavaValue::REFERENCE; vG.val.ref = g;
                        frame->setLocal(1, vG);
                        
                        // Check if already painting
                        if (!paintingThread.expired()) {
                            auto ptr = paintingThread.lock();
                            if (ptr && !ptr->isFinished()) {
                                // Already painting, skip
                                found = true;
                                break;
                            }
                        }

                        auto thread = std::make_shared<JavaThread>(frame);
                        paintingThread = thread;
                        ThreadManager::getInstance().addThread(thread);

                        found = true;
                        break;
                    }
                }
                if (found) break;
                currentCls = currentCls->superClass;
            }
        }
    }
}

} // namespace core
} // namespace j2me
