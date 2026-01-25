#pragma once

#include "JavaThread.hpp"
#include "Diagnostics.hpp"
#include <list>
#include <memory>
#include <chrono>
#include <map>
#include <cstdint>

namespace j2me {
namespace core {

struct ThreadStats {
    size_t total = 0;
    size_t runnable = 0;
    size_t waiting = 0;
    size_t timedWaiting = 0;
    size_t finished = 0;
};

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

        int64_t now = j2me::core::Diagnostics::getInstance().getNowMs();

        for (auto& thread : threads) {
            if (thread && thread->state == JavaThread::TIMED_WAITING && now >= thread->wakeTime) {
                thread->state = JavaThread::RUNNABLE;
                thread->waitingOn = nullptr;
            }
        }

        for (auto it = threads.begin(); it != threads.end(); ++it) {
            auto thread = *it;
            if (!thread) continue;
            if (thread->state == JavaThread::RUNNABLE) {
                threads.erase(it);
                threads.push_back(thread);
                return thread;
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

    ThreadStats getStats() const {
        ThreadStats stats;
        stats.total = threads.size();
        for (const auto& t : threads) {
            if (!t) continue;
            if (t->isFinished()) stats.finished++;
            switch (t->state) {
                case JavaThread::RUNNABLE: stats.runnable++; break;
                case JavaThread::WAITING: stats.waiting++; break;
                case JavaThread::TIMED_WAITING: stats.timedWaiting++; break;
                default: break;
            }
        }
        return stats;
    }

private:
    ThreadManager() = default;
    std::list<std::shared_ptr<JavaThread>> threads;
    std::map<void*, std::shared_ptr<JavaThread>> threadMap;
};

} // namespace core
} // namespace j2me
