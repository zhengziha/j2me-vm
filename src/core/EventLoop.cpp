#include "EventLoop.hpp"
#include "NativeRegistry.hpp"
#include "HeapManager.hpp"
#include "Logger.hpp"
#include "Diagnostics.hpp"
#include "../native/javax_microedition_lcdui_Display.hpp"
#include "../platform/GraphicsContext.hpp"
#include "ThreadManager.hpp"
#include <SDL2/SDL.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>

// some switch buttons
#define JOY_A     0
#define JOY_B     1
#define JOY_X     2
#define JOY_Y     3
#define JOY_PLUS  10
#define JOY_MINUS 11
#define JOY_LEFT  12
#define JOY_UP    13
#define JOY_RIGHT 14
#define JOY_DOWN  15

#define JOY_PAD_LEFT  16
#define JOY_PAD_UP  17
#define JOY_PAD_RIGHT  18
#define JOY_PAD_DOWN  19

namespace j2me {
namespace core {

static int mapKey(SDL_Keycode key) {
    //  LOG_INFO("mapKey: " + std::to_string(key));
    switch (key) {
#ifdef __SWITCH__
        case JOY_A: return -5;
        case JOY_B: return -5;
        case JOY_X: return -6;
        case JOY_Y: return -7;

        case JOY_UP:return -1;
        case JOY_LEFT: return -3;
        case JOY_RIGHT: return -4;
        case JOY_DOWN: return -2;

        case JOY_PAD_LEFT: return 52;
        case JOY_PAD_UP: return 50;
        case JOY_PAD_RIGHT: return 56;
        case JOY_PAD_DOWN: return 54;

        case JOY_MINUS: return -6; // Soft 1 (Left Soft)
        case JOY_PLUS: return -7; // Soft 2 (Right Soft)
#else
        case SDLK_UP: return -1;
        case SDLK_DOWN: return -2;
        case SDLK_LEFT: return -3;
        case SDLK_RIGHT: return -4;
        case SDLK_RETURN: return -5;
        case SDLK_SPACE: return -5;

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

        case SDLK_q: return 49;
        case SDLK_w: return 50;
        case SDLK_e: return 51;
        case SDLK_a: return 52;
        case SDLK_s: return 53;
        case SDLK_d: return 54;
        case SDLK_z: return 55;
        case SDLK_x: return 56;
        case SDLK_c: return 57;
        case SDLK_F1: return -6;// Soft 1 (Left Soft)
        case SDLK_F2: return -7;// Soft 2 (Right Soft)
#endif
//        case SDLK_8: return 42; // STAR
        default: return 0;
    }
}

static int keyStateIndexForKeyCode(int keyCode) {
    switch (keyCode) {
        case -1: return 1;
        case -2: return 6;
        case -3: return 2;
        case -4: return 5;
        case -5: return 8;
        default: return 0;
    }
}

void EventLoop::pollSDL() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        int keyCode = 0;
        if (e.type == SDL_QUIT) {
            requestExit("manual: SDL_QUIT");
#ifdef __SWITCH__
        } else if (e.type == SDL_JOYBUTTONDOWN) {
                keyCode = mapKey(e.jbutton.button);
#else
        } else if (e.type == SDL_KEYDOWN) {
                keyCode = mapKey(e.key.keysym.sym);
#endif
            // LOG_INFO("keyCode: " + std::to_string(keyCode));
            if (keyCode != 0) {
                std::lock_guard<std::mutex> lock(queueMutex);
                eventQueue.push({KeyEvent::PRESSED, keyCode});

                // 更新按键状态位 (用于 GameCanvas)
                // Update keyStates (GameCanvas)
                int idx = keyStateIndexForKeyCode(keyCode);
                if (idx > 0 && idx <= 31) {
                    keyStates |= (1 << idx);
                }
            }
#ifdef __SWITCH__
        } else if (e.type == SDL_JOYBUTTONUP) {
                keyCode = mapKey(e.jbutton.button);
#else
        } else if (e.type == SDL_KEYUP) {
                keyCode = mapKey(e.key.keysym.sym);
#endif
            // LOG_INFO("keyCode: " + std::to_string(keyCode));
            if (keyCode != 0) {
                std::lock_guard<std::mutex> lock(queueMutex);
                eventQueue.push({KeyEvent::RELEASED, keyCode});

                // 更新按键状态位 (用于 GameCanvas)
                int idx = keyStateIndexForKeyCode(keyCode);
                if (idx > 0 && idx <= 31) {
                    keyStates &= ~(1 << idx);
                }
            }
        }
    }

    {
        int64_t nowMs = j2me::core::Diagnostics::getInstance().getNowMs();
        std::lock_guard<std::mutex> lock(queueMutex);
        while (!autoKeyEvents.empty()) {
            const auto& ev = autoKeyEvents.front();
            if (nowMs < ev.atMs) break;
            eventQueue.push({ev.type, ev.keyCode});
            if (ev.type == KeyEvent::PRESSED) {
                LOG_DEBUG("[AutoKey] pressed keyCode=" + std::to_string(ev.keyCode));
            }
            int idx = keyStateIndexForKeyCode(ev.keyCode);
            if (idx > 0 && idx <= 31) {
                if (ev.type == KeyEvent::PRESSED) keyStates |= (1 << idx);
                else if (ev.type == KeyEvent::RELEASED) keyStates &= ~(1 << idx);
            }
            autoKeyEvents.erase(autoKeyEvents.begin());
        }
    }
}

void EventLoop::scheduleAutoKeys(const std::vector<int>& keyCodes, int64_t startDelayMs, int64_t keyPressMs, int64_t betweenKeysMs) {
    if (keyCodes.empty()) return;
    int64_t nowMs = j2me::core::Diagnostics::getInstance().getNowMs();
    std::vector<AutoKeyEvent> newEvents;
    newEvents.reserve(keyCodes.size() * 2);
    int64_t t = nowMs + startDelayMs;
    for (int keyCode : keyCodes) {
        if (keyCode == 0) continue;
        newEvents.push_back({t, KeyEvent::PRESSED, keyCode});
        newEvents.push_back({t + keyPressMs, KeyEvent::RELEASED, keyCode});
        t += betweenKeysMs;
    }
    std::lock_guard<std::mutex> lock(queueMutex);
    autoKeyEvents.insert(autoKeyEvents.end(), newEvents.begin(), newEvents.end());
    std::sort(autoKeyEvents.begin(), autoKeyEvents.end(), [](const AutoKeyEvent& a, const AutoKeyEvent& b) {
        return a.atMs < b.atMs;
    });
    LOG_DEBUG("[AutoKey] scheduled keys=" + std::to_string(keyCodes.size()) + " delayMs=" + std::to_string(startDelayMs));
}

void EventLoop::requestExit(const std::string& reason) {
    bool expected = false;
    if (!quit.compare_exchange_strong(expected, true)) {
        return;
    }

    j2me::platform::GraphicsContext::getInstance().saveFramesBMP("lastframe");

    {
        std::lock_guard<std::mutex> lock(exitMutex);
        exitReason = reason;
    }

    bool logExpected = false;
    if (exitLogged.compare_exchange_strong(logExpected, true)) {
        LOG_INFO("[Exit] Requested: " + reason);
    }
}

std::string EventLoop::getExitReason() const {
    std::lock_guard<std::mutex> lock(exitMutex);
    return exitReason;
}

void EventLoop::dispatchEvents(Interpreter* interpreter) {
    std::queue<KeyEvent> events;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (eventQueue.empty()) return;
        std::swap(events, eventQueue);
    }
    
    while (!events.empty()) {
        KeyEvent event = events.front();
        events.pop();
        
        // 获取当前显示的 Displayable 对象
        j2me::core::JavaObject* displayable = j2me::natives::getCurrentDisplayable();
        if (displayable && displayable->cls) {
            std::string methodName;
            if (event.type == KeyEvent::PRESSED) methodName = "keyPressed";
            else if (event.type == KeyEvent::RELEASED) methodName = "keyReleased";
            
            if (!methodName.empty()) {
                // 向上遍历类层次结构以查找方法
                // Walk up class hierarchy to find method
                auto currentCls = displayable->cls;
                while (currentCls) {
                    bool found = false;
                    for (const auto& method : currentCls->rawFile->methods) {
                        auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(currentCls->rawFile->constant_pool[method.name_index]);
                        if (name->bytes == methodName) {
                            auto frame = std::make_shared<j2me::core::StackFrame>(method, currentCls->rawFile);
                            
                            // 压入 'this'
                            // Push 'this'
                            j2me::core::JavaValue vThis; vThis.type = j2me::core::JavaValue::REFERENCE; vThis.val.ref = displayable;
                            frame->setLocal(0, vThis);
                            
                            // 压入 keyCode
                            // Push keyCode
                            j2me::core::JavaValue vKey; vKey.type = j2me::core::JavaValue::INT; vKey.val.i = event.keyCode;
                            frame->setLocal(1, vKey);
                            
                            // 创建并启动事件处理线程
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
}

// J2ME 规范：处理重绘请求
// Process repaint requests according to J2ME specification
void EventLoop::processRepaints(Interpreter* interpreter) {
    using namespace j2me::natives;

    RepaintRequest& request = getRepaintRequest();
    if (!request.pending) return;

    LOG_DEBUG("[EventLoop] Processing repaint request, pending=true");

    // 调用 serviceRepaints 执行重绘
    serviceRepaints(interpreter);

    // 重绘完成后更新显示
    j2me::platform::GraphicsContext::getInstance().update();

    LOG_DEBUG("[EventLoop] Repaint completed");
}

// 连续渲染：支持依赖此行为的游戏
// Continuous rendering: Support games that depend on this behavior
void EventLoop::continuousPaint(Interpreter* interpreter) {
    using namespace j2me::natives;

    j2me::core::JavaObject* displayable = getCurrentDisplayable();
    if (!displayable || !displayable->cls) return;

    // 检查是否有显式的 repaint 请求 (由游戏调用 repaint() 触发)
    // Check if there's an explicit repaint request (triggered by game calling repaint())
    RepaintRequest& request = getRepaintRequest();

    // 如果有显式请求，让 processRepaints 处理，避免重复绘制
    // If there's an explicit request, let processRepaints handle it to avoid duplicate painting
    if (request.pending) return;

    // 没有显式请求时，提供连续渲染支持
    // When there's no explicit request, provide continuous rendering support
    static std::shared_ptr<j2me::core::JavaClass> graphicsCls;
    if (!graphicsCls) {
        graphicsCls = interpreter->resolveClass("javax/microedition/lcdui/Graphics");
    }

    if (graphicsCls) {
        auto& gc = j2me::platform::GraphicsContext::getInstance();
        gc.beginFrame(); // 清空后缓冲区 / Clear back buffer

        // 分配 Graphics 对象 / Allocate Graphics object
        j2me::core::JavaObject* g = j2me::core::HeapManager::getInstance().allocate(graphicsCls);

        // 初始化 Graphics 对象字段 / Initialize Graphics object fields
        gc.resetClip();
        int w = gc.getWidth();
        int h = gc.getHeight();

        auto setIntField = [&](const std::string& name, int val) {
            auto it = graphicsCls->fieldOffsets.find(name + "|I");
            if (it == graphicsCls->fieldOffsets.end()) it = graphicsCls->fieldOffsets.find(name);
            if (it != graphicsCls->fieldOffsets.end()) {
                g->fields[it->second] = val;
            }
        };

        setIntField("clipX", 0);
        setIntField("clipY", 0);
        setIntField("clipWidth", w);
        setIntField("clipHeight", h);
        setIntField("color", 0); // Black

        // 查找并调用 paint 方法 / Find and call paint method
        auto currentCls = displayable->cls;
        bool paintFound = false;
        while (currentCls) {
            for (const auto& method : currentCls->rawFile->methods) {
                auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(currentCls->rawFile->constant_pool[method.name_index]);
                if (name->bytes == "paint") {
                    auto frame = std::make_shared<j2me::core::StackFrame>(method, currentCls->rawFile);

                    j2me::core::JavaValue vThis;
                    vThis.type = j2me::core::JavaValue::REFERENCE;
                    vThis.val.ref = displayable;
                    frame->setLocal(0, vThis);

                    j2me::core::JavaValue vG;
                    vG.type = j2me::core::JavaValue::REFERENCE;
                    vG.val.ref = g;
                    frame->setLocal(1, vG);

                    // 执行 paint 方法 / Execute paint method
                    auto paintThread = std::make_shared<core::JavaThread>(frame);
                    interpreter->execute(paintThread, 100000);
                    paintFound = true;
                    break;
                }
            }
            if (paintFound) break;
            currentCls = currentCls->superClass;
        }

        // paint 完成后提交并显示 / Commit and update after paint completes
        gc.commit();
        gc.update();
    }
}

} // namespace core
} // namespace j2me
