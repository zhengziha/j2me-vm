#pragma once

#include "Interpreter.hpp"
#include "JavaThread.hpp"
#include <queue>
#include <mutex>
#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace j2me {
namespace core {

// Key Event structure
// 按键事件结构体
struct KeyEvent {
    enum Type { PRESSED, RELEASED }; // 按下或释放
    Type type;    // 事件类型
    int keyCode;  // 键码 (J2ME Key Code)
};

class EventLoop {
public:
    // Singleton access
    // 获取单例实例
    static EventLoop& getInstance() {
        static EventLoop instance;
        return instance;
    }

    // Called by Main Thread
    // 由主线程调用: 轮询 SDL 事件并将按键事件放入队列
    void pollSDL(); 
    
    // Called by VM Thread
    // 由虚拟机线程调用: 分发事件到 Java 层 (调用 keyPressed/keyReleased)
    void dispatchEvents(Interpreter* interpreter);
    
    // 渲染画面 (调用当前 Displayable 的 paint 方法)
    void render(Interpreter* interpreter);
    
    // 检查绘制线程是否完成，并提交帧缓冲区
    void checkPaintFinished();
    
    void requestExit(const std::string& reason);
    std::string getExitReason() const;

    void scheduleAutoKeys(const std::vector<int>& keyCodes, int64_t startDelayMs = 1200, int64_t keyPressMs = 40, int64_t betweenKeysMs = 200);

    // 检查是否收到退出请求
    bool shouldExit() const { return quit; }
    
    // 获取当前按键状态位掩码 (用于 GameCanvas)
    int getKeyStates() const { return keyStates; }

    // Legacy static wrappers (deprecated/modified)
    // 旧的静态包装器 (已弃用/修改)，为了兼容性保留
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
    struct AutoKeyEvent {
        int64_t atMs;
        KeyEvent::Type type;
        int keyCode;
    };

    std::queue<KeyEvent> eventQueue; // Stores mapped keyCodes with type / 存储按键事件的队列
    std::mutex queueMutex;           // Protects eventQueue / 保护事件队列的互斥锁
    std::atomic<bool> quit{false};   // Exit flag / 退出标志
    std::atomic<int> keyStates{0};   // Bitmask of pressed keys / 按键状态位掩码
    std::atomic<bool> exitLogged{false};
    mutable std::mutex exitMutex;
    std::string exitReason;
    std::vector<AutoKeyEvent> autoKeyEvents;
    
    // Track current painting thread to avoid flooding
    // 跟踪当前的绘制线程，避免绘制请求堆积
    std::weak_ptr<JavaThread> paintingThread;
    bool isPainting = false; // 是否正在绘制中
    uint64_t lastPaintCommittedDrawCount = 0;
    int64_t lastPaintPartialCommitMs = 0;
};

} // namespace core
} // namespace j2me
