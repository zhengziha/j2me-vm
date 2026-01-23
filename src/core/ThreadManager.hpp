#pragma once

#include "JavaThread.hpp"
#include <list>
#include <memory>
#include <chrono>
#include <map>

namespace j2me {
namespace core {

class ThreadManager {
public:
    static ThreadManager& getInstance() {
        static ThreadManager instance;
        return instance;
    }

    void addThread(std::shared_ptr<JavaThread> thread) {
        threads.push_back(thread);
    }
    
    void registerThread(void* javaObj, std::shared_ptr<JavaThread> thread) {
        if (javaObj) {
            thread->javaThreadObject = javaObj;
            threadMap[javaObj] = thread;
        }
        addThread(thread);
    }

    std::shared_ptr<JavaThread> getJavaThread(void* javaObj) {
        auto it = threadMap.find(javaObj);
        if (it != threadMap.end()) return it->second;
        return nullptr;
    }
    
    void notify(void* obj) {
        for (auto& thread : threads) {
            if ((thread->state == JavaThread::WAITING || thread->state == JavaThread::TIMED_WAITING) && thread->waitingOn == obj) {
                thread->state = JavaThread::RUNNABLE;
                thread->waitingOn = nullptr;
                return; // Notify one
            }
        }
    }
    
    void notifyAll(void* obj) {
         for (auto& thread : threads) {
            if ((thread->state == JavaThread::WAITING || thread->state == JavaThread::TIMED_WAITING) && thread->waitingOn == obj) {
                thread->state = JavaThread::RUNNABLE;
                thread->waitingOn = nullptr;
            }
        }
    }

    std::shared_ptr<JavaThread> nextThread() {
        if (threads.empty()) return nullptr;

        // Simple Round Robin
        // Move current front to back if it's not finished
        // But we need to handle waiting threads
        
        // Find first RUNNABLE thread
        for (auto it = threads.begin(); it != threads.end(); ++it) {
            auto thread = *it;
            if (thread->state == JavaThread::RUNNABLE) {
                // Move this thread to back to ensure fairness (Round Robin)
                threads.erase(it);
                threads.push_back(thread);
                return thread;
            } else if (thread->state == JavaThread::TIMED_WAITING) {
                // Check if time is up
                auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
                ).count();
                
                if (now >= thread->wakeTime) {
                    thread->state = JavaThread::RUNNABLE;
                    threads.erase(it);
                    threads.push_back(thread);
                    return thread;
                }
            }
        }
        
        return nullptr;
    }
    
    void removeFinishedThreads() {
        threads.remove_if([this](const std::shared_ptr<JavaThread>& t) {
            if (t->isFinished()) {
                if (t->javaThreadObject) {
                    notifyAll(t->javaThreadObject);
                    threadMap.erase(t->javaThreadObject);
                }
                return true;
            }
            return false;
        });
    }

    bool hasThreads() const {
        return !threads.empty();
    }

private:
    ThreadManager() = default;
    std::list<std::shared_ptr<JavaThread>> threads;
    std::map<void*, std::shared_ptr<JavaThread>> threadMap;
};

} // namespace core
} // namespace j2me
