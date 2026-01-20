#include "StackFrame.hpp"
#include <stdexcept>
#include <iostream>

namespace j2me {
namespace core {

StackFrame::StackFrame(const MethodInfo& method, const std::shared_ptr<ClassFile>& classFile)
    : method(method), classFile(classFile) {
    localVariables.resize(20); 
    operandStack.reserve(20);
}

void StackFrame::push(JavaValue value) {
    operandStack.push_back(value);
}

JavaValue StackFrame::pop() {
    if (operandStack.empty()) {
        throw std::runtime_error("Stack underflow");
    }
    JavaValue val = operandStack.back();
    operandStack.pop_back();
    return val;
}

JavaValue StackFrame::peek() {
    if (operandStack.empty()) {
        throw std::runtime_error("Stack underflow");
    }
    return operandStack.back();
}

void StackFrame::setLocal(uint16_t index, JavaValue value) {
    if (index >= localVariables.size()) {
        localVariables.resize(index + 1);
    }
    localVariables[index] = value;
}

JavaValue StackFrame::getLocal(uint16_t index) {
    if (index >= localVariables.size()) {
        // Return default 0/null
        JavaValue v;
        v.type = JavaValue::INT;
        v.val.i = 0;
        return v;
    }
    return localVariables[index];
}

} // namespace core
} // namespace j2me
