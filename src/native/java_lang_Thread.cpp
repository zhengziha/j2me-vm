#include "java_lang_Thread.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/JavaThread.hpp"
#include "../core/ThreadManager.hpp"
#include "../core/RuntimeTypes.hpp"
#include "../core/Diagnostics.hpp"
#include <chrono>
#include <iostream>

namespace j2me {
namespace natives {

void registerThreadNatives(j2me::core::NativeRegistry& registry) {
    // java/lang/Thread.start0()V
    registry.registerNative("java/lang/Thread", "start0", "()V",
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisObj = frame->pop(); // this Thread object
            if (thisObj.val.ref == nullptr) {
                std::cerr << "NullPointerException in Thread.start0" << std::endl;
                return;
            }
            j2me::core::JavaObject* threadObj = (j2me::core::JavaObject*)thisObj.val.ref;
            
            // Create new JavaThread
            // We need an initial frame.
            // The initial frame should be run() method of the threadObj's class.
            
            auto cls = threadObj->cls;
            // Find run()V
             bool found = false;
             j2me::core::MethodInfo runMethod;
             std::shared_ptr<j2me::core::ClassFile> methodClassFile;
             
             // Virtual lookup for run()
             std::shared_ptr<j2me::core::JavaClass> current = cls;
             while (current) {
                 for (const auto& method : current->rawFile->methods) {
                     auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(current->rawFile->constant_pool[method.name_index]);
                     auto desc = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(current->rawFile->constant_pool[method.descriptor_index]);
                     if (name && desc && name->bytes == "run" && desc->bytes == "()V") {
                         runMethod = method;
                         methodClassFile = current->rawFile;
                         found = true;
                         break;
                     }
                 }
                 if (found) break;
                 current = current->superClass;
             }
             
             if (!found) {
                 std::cerr << "Could not find run() method in Thread class" << std::endl;
                 return;
             }
             
             auto newFrame = std::make_shared<j2me::core::StackFrame>(runMethod, methodClassFile);
             // push 'this' as argument 0
             j2me::core::JavaValue thisVal; 
             thisVal.type = j2me::core::JavaValue::REFERENCE; 
             thisVal.val.ref = threadObj;
             newFrame->setLocal(0, thisVal);
             
             auto newThread = std::make_shared<j2me::core::JavaThread>(newFrame);
             j2me::core::ThreadManager::getInstance().registerThread(threadObj, newThread);
        }
    );

    // java/lang/Thread.start()V
    registry.registerNative("java/lang/Thread", "start", "()V",
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
             // Just delegate to start0
             auto start0 = registry.getNative("java/lang/Thread", "start0", "()V");
             if (start0) {
                 start0(thread, frame);
             } else {
                 std::cerr << "Native start0 not found" << std::endl;
             }
        }
    );

    // java/lang/Thread.yield()V
    registry.registerNative("java/lang/Thread", "yield", "()V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            // Just yield to scheduler
            // No-op as round-robin will switch eventually
        }
    );

    // java/lang/Thread.sleep(J)V
    registry.registerNative("java/lang/Thread", "sleep", "(J)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int64_t millis = frame->pop().val.l;
            if (millis < 0) return; 
            
            if (millis == 0) {
                return;
            }
            
            thread->state = j2me::core::JavaThread::TIMED_WAITING;
            int64_t now = j2me::core::Diagnostics::getInstance().getNowMs();
            thread->wakeTime = now + millis;
        }
    );
}

} // namespace natives
} // namespace j2me
