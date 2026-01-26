#include "java_lang_Double.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include <cstring>
#include <cmath>
#include <sstream>

namespace j2me {
namespace natives {

void registerDoubleNatives(j2me::core::NativeRegistry& registry) {
    registry.registerNative("java/lang/Double", "doubleToLongBits", "(D)J", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            double value = frame->pop().val.d;
            int64_t bits;
            std::memcpy(&bits, &value, sizeof(double));
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::LONG;
            result.val.l = bits;
            frame->push(result);
        });

    registry.registerNative("java/lang/Double", "longBitsToDouble", "(J)D", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int64_t bits = frame->pop().val.l;
            double value;
            std::memcpy(&value, &bits, sizeof(double));
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::DOUBLE;
            result.val.d = value;
            frame->push(result);
        });

    registry.registerNative("java/lang/Double", "toStringNative", "(D)Ljava/lang/String;", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            double value = frame->pop().val.d;
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
