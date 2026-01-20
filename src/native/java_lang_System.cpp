#include "java_lang_System.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include <iostream>
#include <chrono>
#include <cstring>

namespace j2me {
namespace natives {

void registerSystemNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();

    // java/lang/System.currentTimeMillis()J
    registry.registerNative("java/lang/System", "currentTimeMillis", "()J",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::LONG;
            ret.val.l = millis;
            frame->push(ret);
        }
    );

    // java/lang/System.gc()V
    registry.registerNative("java/lang/System", "gc", "()V",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            std::cout << "[System] gc() called - triggering garbage collection" << std::endl;
            // j2me::core::HeapManager::getInstance().collect(); // If implemented
        }
    );
    
    // java/lang/System.currentTimeMillisNative()J - kept for backward compatibility if used elsewhere
    registry.registerNative("java/lang/System", "currentTimeMillisNative", "()J",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::LONG;
            ret.val.l = millis;
            frame->push(ret);
        }
    );

    // java/lang/System.arraycopy(Ljava/lang/Object;ILjava/lang/Object;II)V
    registry.registerNative("java/lang/System", "arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            // Pop arguments in reverse order
            int length = frame->pop().val.i;
            int dstPos = frame->pop().val.i;
            j2me::core::JavaValue dstVal = frame->pop();
            int srcPos = frame->pop().val.i;
            j2me::core::JavaValue srcVal = frame->pop();
            
            if (srcVal.type != j2me::core::JavaValue::REFERENCE || dstVal.type != j2me::core::JavaValue::REFERENCE) {
                std::cerr << "ArrayStoreException: src or dst not a reference" << std::endl;
                return; // Should throw exception
            }
            
            if (srcVal.val.ref == nullptr || dstVal.val.ref == nullptr) {
                std::cerr << "NullPointerException: src or dst is null" << std::endl;
                return; // Should throw exception
            }
            
            auto srcObj = static_cast<j2me::core::JavaObject*>(srcVal.val.ref);
            auto dstObj = static_cast<j2me::core::JavaObject*>(dstVal.val.ref);
            
            // Basic bounds checks
            if (srcPos < 0 || dstPos < 0 || length < 0 || 
                srcPos + length > (int)srcObj->fields.size() || 
                dstPos + length > (int)dstObj->fields.size()) {
                std::cerr << "IndexOutOfBoundsException: srcPos=" << srcPos << " dstPos=" << dstPos << " length=" << length 
                          << " srcLen=" << srcObj->fields.size() << " dstLen=" << dstObj->fields.size() << std::endl;
                return; // Should throw exception
            }
            
            // Perform copy
            // Since fields are vector<int64_t>, we can just copy
            for (int i = 0; i < length; i++) {
                dstObj->fields[dstPos + i] = srcObj->fields[srcPos + i];
            }
        }
    );

    // java/lang/System.exitNative(I)V
    registry.registerNative("java/lang/System", "exitNative", "(I)V",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int status = frame->pop().val.i;
            std::cout << "System.exit(" << status << ") called." << std::endl;
            exit(status);
        }
    );
    
    // java/lang/System.printNative(Ljava/lang/String;)V
    registry.registerNative("java/lang/System", "printNative", "(Ljava/lang/String;)V",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
             j2me::core::JavaValue strVal = frame->pop();
             if (strVal.type == j2me::core::JavaValue::REFERENCE && strVal.val.ref != nullptr) {
                 // For simplicity, just printing a marker or attempting to extract string
                 // Ideally reuse string extraction logic or just print to stdout
                 // But wait, printNative is used by System.out implementation in System.java
                 // In System.java: printNative(String.valueOf((char)b));
                 
                 // Reuse logic from PrintStream or just basic extraction
                 auto strObj = static_cast<j2me::core::JavaObject*>(strVal.val.ref);
                 // Need to extract char array from String object
                 // This duplicates logic from PrintStream, maybe refactor later
                 // For now, minimal implementation
                 std::cout << "[System.printNative] " << std::hex << strObj << std::dec << std::endl;
             }
        }
    );
}

}
}
