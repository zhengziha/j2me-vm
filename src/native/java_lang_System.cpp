#include "java_lang_System.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "java_lang_String.hpp"
#include <iostream>
#include <chrono>
#include <cstring>

namespace j2me {
namespace natives {

void registerSystemNatives(j2me::core::NativeRegistry& registry) {
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
            // Relaxed check: Clip length if out of bounds (for buggy games)
            bool clipped = false;
            int originalLength = length;
            
            if (srcPos < 0 || dstPos < 0 || length < 0) {
                 std::cerr << "ArrayStoreException: negative args" << std::endl;
                 return;
            }
            
            if (srcPos + length > (int)srcObj->fields.size()) {
                length = (int)srcObj->fields.size() - srcPos;
                clipped = true;
            }
            
            if (dstPos + length > (int)dstObj->fields.size()) {
                length = (int)dstObj->fields.size() - dstPos;
                clipped = true;
            }
            
            if (clipped) {
                std::cout << "[System] arraycopy clipped length from " << originalLength << " to " << length 
                          << " srcLen=" << srcObj->fields.size() << " dstLen=" << dstObj->fields.size() << std::endl;
            }
            
            /*
            if (srcPos < 0 || dstPos < 0 || length < 0 || 
                srcPos + length > (int)srcObj->fields.size() || 
                dstPos + length > (int)dstObj->fields.size()) {
                std::cerr << "IndexOutOfBoundsException: srcPos=" << srcPos << " dstPos=" << dstPos << " length=" << length 
                          << " srcLen=" << srcObj->fields.size() << " dstLen=" << dstObj->fields.size() << std::endl;
                return; // Should throw exception
            }
            */
            
            // Perform copy
            // Since fields are vector<int64_t>, we can just copy
            // BUT must handle overlapping buffers (memmove semantics)
            
            if (srcObj == dstObj && dstPos > srcPos) {
                // Copy backwards
                for (int i = length - 1; i >= 0; i--) {
                    dstObj->fields[dstPos + i] = srcObj->fields[srcPos + i];
                }
            } else {
                // Copy forwards
                for (int i = 0; i < length; i++) {
                    dstObj->fields[dstPos + i] = srcObj->fields[srcPos + i];
                }
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
    auto printImpl = [](std::shared_ptr<j2me::core::StackFrame> frame) {
             j2me::core::JavaValue strVal = frame->pop();
             if (frame->method.name_index > 0) { // Check if it's static or virtual
                 // System.printNative is static (no this)
                 // PrintStream.printNative is instance (has this)
                 // But wait, lambda doesn't know.
                 // We need to know signature.
                 // Actually, if we register same lambda for both, we need to handle 'this' if present.
             }
             // However, simpler: define separate lambdas or pop 'this' if needed.
             // System.printNative is static.
             // PrintStream.printNative is instance (private native).
             // If it's instance method, 'this' is on stack below arguments.
    };

    registry.registerNative("java/lang/System", "printNative", "(Ljava/lang/String;)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
             j2me::core::JavaValue strVal = frame->pop();
             if (strVal.type == j2me::core::JavaValue::REFERENCE && strVal.val.ref != nullptr) {
                 auto strObj = static_cast<j2me::core::JavaObject*>(strVal.val.ref);
                 std::string s = getJavaString(strObj);
                 std::cout << s; 
             }
        }
    );

    // java/io/PrintStream.printNative(Ljava/lang/String;)V
    registry.registerNative("java/io/PrintStream", "printNative", "(Ljava/lang/String;)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
             j2me::core::JavaValue strVal = frame->pop();
             frame->pop(); // this
             if (strVal.type == j2me::core::JavaValue::REFERENCE && strVal.val.ref != nullptr) {
                 auto strObj = static_cast<j2me::core::JavaObject*>(strVal.val.ref);
                 std::string s = getJavaString(strObj);
                 std::cout << s; 
             }
        }
    );
}

}
}
