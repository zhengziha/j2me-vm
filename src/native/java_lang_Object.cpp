#include "java_lang_Object.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "../core/ThreadManager.hpp"
#include "../core/Diagnostics.hpp"
#include "java_lang_String.hpp"
#include <iostream>
#include <chrono>

namespace j2me {
namespace natives {

void registerObjectNatives(j2me::core::NativeRegistry& registry) {
    // registry passed as argument

    // java/lang/Object.getClass()Ljava/lang/Class;
    registry.registerNative("java/lang/Object", "getClass", "()Ljava/lang/Class;", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue objVal = frame->pop(); // this
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::REFERENCE;
            ret.val.ref = nullptr;
            
            if (objVal.val.ref == nullptr) {
                std::cerr << "NullPointerException in Object.getClass()" << std::endl;
            } else {
                j2me::core::JavaObject* obj = static_cast<j2me::core::JavaObject*>(objVal.val.ref);
                
                auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                if (interpreter) {
                    auto classCls = interpreter->resolveClass("java/lang/Class");
                    if (classCls) {
                        auto classObj = j2me::core::HeapManager::getInstance().allocate(classCls);
                        
                        // Set 'name' field
                        std::string className = obj->cls->name;
                        // Convert internal name (slashes) to binary name (dots)
                        for (auto& c : className) {
                            if (c == '/') c = '.';
                        }
                        
                        auto stringObj = createJavaString(interpreter, className);
                        
                        auto nameIt = classCls->fieldOffsets.find("name|Ljava/lang/String;");
                        if (nameIt == classCls->fieldOffsets.end()) {
                            nameIt = classCls->fieldOffsets.find("name");
                        }
                        if (nameIt != classCls->fieldOffsets.end()) {
                            classObj->fields[nameIt->second] = (int64_t)stringObj;
                        }
                        
                        ret.val.ref = classObj;
                        // std::cout << "[Object] getClass returning Class object for " << className << std::endl;
                    } else {
                        std::cerr << "[Object] Failed to resolve java/lang/Class" << std::endl;
                    }
                }
            }
            frame->push(ret);
        }
    );

    // java/lang/Object.hashCode()I
    registry.registerNative("java/lang/Object", "hashCode", "()I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue objVal = frame->pop(); // this
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            // Use pointer address as hash code
            ret.val.i = (int32_t)(intptr_t)objVal.val.ref;
            frame->push(ret);
        }
    );

    // java/lang/Object.wait(J)V
    registry.registerNative("java/lang/Object", "wait", "(J)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
             int64_t timeout = frame->pop().val.l;
             j2me::core::JavaValue thisObj = frame->pop(); // this
             
             if (thisObj.val.ref == nullptr) {
                 // Throw NPE - for now just return
                 std::cerr << "NullPointerException in Object.wait" << std::endl;
                 return;
             }
             
             if (timeout < 0) {
                 std::cerr << "IllegalArgumentException in Object.wait: timeout < 0" << std::endl;
                 return;
             }
             
             if (timeout == 0) {
                 thread->state = j2me::core::JavaThread::WAITING;
                 thread->wakeTime = 0;
             } else {
                 thread->state = j2me::core::JavaThread::TIMED_WAITING;
                 int64_t now = j2me::core::Diagnostics::getInstance().getNowMs();
                 thread->wakeTime = now + timeout;
             }
             
             thread->waitingOn = thisObj.val.ref;
        }
    );

    // java/lang/Object.notify()V
    registry.registerNative("java/lang/Object", "notify", "()V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
             j2me::core::JavaValue thisObj = frame->pop(); // this
             if (thisObj.val.ref == nullptr) return; // NPE
             
             j2me::core::ThreadManager::getInstance().notify(thisObj.val.ref);
        }
    );

    // java/lang/Object.notifyAll()V
    registry.registerNative("java/lang/Object", "notifyAll", "()V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
             j2me::core::JavaValue thisObj = frame->pop(); // this
             if (thisObj.val.ref == nullptr) return; // NPE
             
             j2me::core::ThreadManager::getInstance().notifyAll(thisObj.val.ref);
        }
    );
}

} // namespace natives
} // namespace j2me
