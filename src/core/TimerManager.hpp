#pragma once

#include "RuntimeTypes.hpp"
#include "Interpreter.hpp"
#include "ThreadManager.hpp"
#include <vector>
#include <algorithm>
#include <chrono>

namespace j2me {
namespace core {

struct TimerTaskEntry {
    JavaObject* task;
    int64_t nextRunTime; // Milliseconds since epoch
    int64_t period;      // 0 = one-shot, >0 = fixed-rate, <0 = fixed-delay
    bool scheduled;
};

class TimerManager {
public:
    static TimerManager& getInstance() {
        static TimerManager instance;
        return instance;
    }

    void schedule(JavaObject* task, int64_t delay, int64_t period) {
        using namespace std::chrono;
        auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        
        TimerTaskEntry entry;
        entry.task = task;
        entry.nextRunTime = now + delay;
        entry.period = period;
        entry.scheduled = true;
        
        tasks.push_back(entry);
    }

    void tick(Interpreter* interpreter) {
        using namespace std::chrono;
        auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        
        std::vector<TimerTaskEntry> tasksToRun;
        
        // 1. Identify tasks to run
        for (auto& entry : tasks) {
            if (entry.scheduled && now >= entry.nextRunTime) {
                tasksToRun.push_back(entry);
                
                // Update next run time
                if (entry.period == 0) {
                    entry.scheduled = false; // One-shot
                } else if (entry.period > 0) {
                    // Fixed-rate
                    entry.nextRunTime += entry.period;
                } else {
                    // Fixed-delay
                    entry.nextRunTime = now - entry.period;
                }
            }
        }
        
        // 2. Remove non-scheduled tasks
        tasks.erase(std::remove_if(tasks.begin(), tasks.end(), 
            [](const TimerTaskEntry& e) { return !e.scheduled; }), tasks.end());
            
        // 3. Execute tasks
        for (auto& entry : tasksToRun) {
            executeRun(interpreter, entry.task);
        }
    }
    
private:
    std::vector<TimerTaskEntry> tasks;
    
    void executeRun(Interpreter* interpreter, JavaObject* task) {
        if (!task || !task->cls) return;
        
        auto currentCls = task->cls;
        while (currentCls) {
            bool found = false;
            for (const auto& method : currentCls->rawFile->methods) {
                auto name = std::dynamic_pointer_cast<ConstantUtf8>(currentCls->rawFile->constant_pool[method.name_index]);
                auto desc = std::dynamic_pointer_cast<ConstantUtf8>(currentCls->rawFile->constant_pool[method.descriptor_index]);
                
                if (name && desc && name->bytes == "run" && desc->bytes == "()V") {
                    auto frame = std::make_shared<StackFrame>(method, currentCls->rawFile);
                    JavaValue vThis; vThis.type = JavaValue::REFERENCE; vThis.val.ref = task;
                    frame->setLocal(0, vThis);
                    
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
};

}
}
