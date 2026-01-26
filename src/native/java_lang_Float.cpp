#include "java_lang_Float.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include <cstring>
#include <cmath>
#include <sstream>

namespace j2me {
namespace natives {

void registerFloatNatives(j2me::core::NativeRegistry& registry) {
    registry.registerNative("java/lang/Float", "floatToIntBits", "(F)I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            float value = frame->pop().val.f;
            int32_t bits;
            std::memcpy(&bits, &value, sizeof(float));
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = bits;
            frame->push(result);
        });

    registry.registerNative("java/lang/Float", "intBitsToFloat", "(I)F", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int32_t bits = frame->pop().val.i;
            float value;
            std::memcpy(&value, &bits, sizeof(float));
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::FLOAT;
            result.val.f = value;
            frame->push(result);
        });

    registry.registerNative("java/lang/Float", "toStringNative", "(F)Ljava/lang/String;", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            float value = frame->pop().val.f;
            std::ostringstream oss;
            oss << value;
            std::string str = oss.str();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.strVal = str;
            result.val.ref = nullptr;
            frame->push(result);
        });
}

} 
}
