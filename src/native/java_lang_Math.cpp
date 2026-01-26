#include "java_lang_Math.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include <cmath>

namespace j2me {
namespace natives {

void registerMathNatives(j2me::core::NativeRegistry& registry) {
    registry.registerNative("java/lang/Math", "sin", "(D)D", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            double value = frame->pop().val.d;
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::DOUBLE;
            result.val.d = std::sin(value);
            frame->push(result);
        });

    registry.registerNative("java/lang/Math", "cos", "(D)D", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            double value = frame->pop().val.d;
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::DOUBLE;
            result.val.d = std::cos(value);
            frame->push(result);
        });

    registry.registerNative("java/lang/Math", "tan", "(D)D", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            double value = frame->pop().val.d;
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::DOUBLE;
            result.val.d = std::tan(value);
            frame->push(result);
        });

    registry.registerNative("java/lang/Math", "sqrt", "(D)D", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            double value = frame->pop().val.d;
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::DOUBLE;
            result.val.d = std::sqrt(value);
            frame->push(result);
        });

    registry.registerNative("java/lang/Math", "ceil", "(D)D", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            double value = frame->pop().val.d;
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::DOUBLE;
            result.val.d = std::ceil(value);
            frame->push(result);
        });

    registry.registerNative("java/lang/Math", "floor", "(D)D", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            double value = frame->pop().val.d;
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::DOUBLE;
            result.val.d = std::floor(value);
            frame->push(result);
        });
}

} 
}
