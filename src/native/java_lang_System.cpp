#include "java_lang_System.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Logger.hpp"
#include "java_lang_String.hpp"
#include <chrono>
#include <cstring>

namespace j2me {
namespace natives {

void registerSystemNatives(j2me::core::NativeRegistry& registry) {
    // java/lang/System.currentTimeMillis()J
    registry.registerNative("java/lang/System", "currentTimeMillis", "()J",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            static auto start = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::LONG;
            ret.val.l = millis;
            frame->push(ret);
        }
    );

    // java/lang/System.gc()V
    registry.registerNative("java/lang/System", "gc", "()V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            LOG_DEBUG("[System] gc() called - triggering garbage collection");
            // j2me::core::HeapManager::getInstance().collect(); // If implemented
        }
    );
    
    // java/lang/System.currentTimeMillisNative()J - kept for backward compatibility if used elsewhere
    registry.registerNative("java/lang/System", "currentTimeMillisNative", "()J",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            static auto start = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::LONG;
            ret.val.l = millis;
            frame->push(ret);
        }
    );

    // java/lang/System.arraycopy(Ljava/lang/Object;ILjava/lang/Object;II)V
    registry.registerNative("java/lang/System", "arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            // Pop arguments in reverse order
            int length = frame->pop().val.i;
            int dstPos = frame->pop().val.i;
            j2me::core::JavaValue dstVal = frame->pop();
            int srcPos = frame->pop().val.i;
            j2me::core::JavaValue srcVal = frame->pop();
            
            if (srcVal.type != j2me::core::JavaValue::REFERENCE || dstVal.type != j2me::core::JavaValue::REFERENCE) {
                LOG_ERROR("ArrayStoreException: src or dst not a reference");
                return; // Should throw exception
            }
            
            if (srcVal.val.ref == nullptr || dstVal.val.ref == nullptr) {
                LOG_ERROR("NullPointerException: src or dst is null");
                return; // Should throw exception
            }
            
            auto srcObj = static_cast<j2me::core::JavaObject*>(srcVal.val.ref);
            auto dstObj = static_cast<j2me::core::JavaObject*>(dstVal.val.ref);
            
            // Basic bounds checks
            // Relaxed check: Clip length if out of bounds (for buggy games)
            bool clipped = false;
            int originalLength = length;
            
            if (srcPos < 0 || dstPos < 0 || length < 0) {
                 LOG_ERROR("ArrayStoreException: negative args");
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
                LOG_DEBUG("[System] arraycopy clipped length from " + std::to_string(originalLength) + " to " + std::to_string(length) + " srcLen=" + std::to_string(srcObj->fields.size()) + " dstLen=" + std::to_string(dstObj->fields.size()));
            }
            
            /*
            if (srcPos < 0 || dstPos < 0 || length < 0 || 
                srcPos + length > (int)srcObj->fields.size() || 
                dstPos + length > (int)dstObj->fields.size()) {
                LOG_ERROR("IndexOutOfBoundsException: srcPos=" + std::to_string(srcPos) + " dstPos=" + std::to_string(dstPos) + " length=" + std::to_string(length) + " srcLen=" + std::to_string(srcObj->fields.size()) + " dstLen=" + std::to_string(dstObj->fields.size()));
                return; // Should throw exception
            }
            */
            
            // Perform copy
            // Since fields are vector<int64_t>, we can just copy
            // BUT must handle overlapping buffers (memmove semantics)
            
            // Debugging for ByteArrayInputStream crash
            LOG_DEBUG("[System] arraycopy: src=" + std::to_string((uintptr_t)srcObj) + " dst=" + std::to_string((uintptr_t)dstObj) + " srcPos=" + std::to_string(srcPos) + " dstPos=" + std::to_string(dstPos) + " len=" + std::to_string(length));
            if (srcObj) LOG_DEBUG("  src.fields.size=" + std::to_string(srcObj->fields.size()));
            if (dstObj) LOG_DEBUG("  dst.fields.size=" + std::to_string(dstObj->fields.size()));

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
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int status = frame->pop().val.i;
            LOG_INFO("System.exit(" + std::to_string(status) + ") called.");
            exit(status);
        }
    );
    
    // java/lang/System.printNative(Ljava/lang/String;)V
    auto printImpl = [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
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
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
             j2me::core::JavaValue strVal = frame->pop();
             if (strVal.type == j2me::core::JavaValue::REFERENCE && strVal.val.ref != nullptr) {
                 auto strObj = static_cast<j2me::core::JavaObject*>(strVal.val.ref);
                 std::string s = getJavaString(strObj);
                 LOG_INFO(s);
             }
        }
    );

    // java/io/PrintStream.printNative(Ljava/lang/String;)V
    registry.registerNative("java/io/PrintStream", "printNative", "(Ljava/lang/String;)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
             j2me::core::JavaValue strVal = frame->pop();
             frame->pop(); // this
             if (strVal.type == j2me::core::JavaValue::REFERENCE && strVal.val.ref != nullptr) {
                 auto strObj = static_cast<j2me::core::JavaObject*>(strVal.val.ref);
                 std::string s = getJavaString(strObj);
                 LOG_INFO(s); 
             }
        }
    );
}

}
}
