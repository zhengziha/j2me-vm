#include "StackFrame.hpp"
#include "../util/DataReader.hpp"
#include <stdexcept>
#include <iostream>

namespace j2me {
namespace core {

StackFrame::StackFrame(const MethodInfo& method, const std::shared_ptr<ClassFile>& classFile)
    : method(method), classFile(classFile) {
    localVariables.resize(20); 
    operandStack.reserve(20);

    // Parse Code attribute
    for (const auto& attr : method.attributes) {
        if (attr.attribute_name_index < classFile->constant_pool.size()) {
            auto nameInfo = std::dynamic_pointer_cast<ConstantUtf8>(classFile->constant_pool[attr.attribute_name_index]);
            if (nameInfo && nameInfo->bytes == "Code") {
                 util::DataReader attrReader(attr.info);
                 attrReader.readU2(); // max_stack
                 attrReader.readU2(); // max_locals
                 uint32_t codeLength = attrReader.readU4();
                 code = attrReader.readBytes(codeLength);
                 break;
            }
        }
    }
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
