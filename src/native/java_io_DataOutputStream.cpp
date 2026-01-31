#include "java_io_DataOutputStream.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "../core/Logger.hpp"
#include "../loader/JarLoader.hpp"
#include "NativeInputStream.hpp"
#include <string>
#include <vector>

namespace j2me {
namespace natives {

void registerDataOutputStreamNatives(j2me::core::NativeRegistry& registry) {
    // java/io/DataOutputStream.size()I
    registry.registerNative("java/io/DataOutputStream", "size", "()I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = 0;
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* dataOutputStreamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                
                LOG_DEBUG("[DataOutputStream.size()I] BEFORE - object: " + std::to_string((intptr_t)dataOutputStreamObj));
                
                // Assuming field index 1 is the 'written' field (based on the Java class definition)
                // Field layout: out(OutputStream) at index 0, written(int) at index 1
                if (dataOutputStreamObj->fields.size() > 1) {
                    result.val.i = (int)dataOutputStreamObj->fields[1];
                    LOG_DEBUG("[DataOutputStream.size()I] AFTER - object: " + std::to_string((intptr_t)dataOutputStreamObj) + " size: " + std::to_string(result.val.i));
                } else {
                    LOG_ERROR("[DataOutputStream.size()I] ERROR - object has insufficient fields");
                }
            } else {
                LOG_ERROR("[DataOutputStream.size()I] ERROR - Invalid this object");
            }
            
            frame->push(result);
        }
    );
}

} // namespace natives
} // namespace j2me