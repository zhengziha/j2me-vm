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

    // 解析 Code 属性
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
                 
                 uint16_t exceptionTableLength = attrReader.readU2();
                 for (int i = 0; i < exceptionTableLength; i++) {
                     ExceptionTableEntry entry;
                     entry.startPc = attrReader.readU2();
                     entry.endPc = attrReader.readU2();
                     entry.handlerPc = attrReader.readU2();
                     entry.catchType = attrReader.readU2();
                     exceptionTable.push_back(entry);
                 }
                 
                 // Parse sub-attributes (LineNumberTable)
                 // if (attrReader.canRead(2)) {
                     uint16_t subAttributesCount = attrReader.readU2();
                     for (int i = 0; i < subAttributesCount; i++) {
                         uint16_t subAttrNameIndex = attrReader.readU2();
                         uint32_t subAttrLength = attrReader.readU4();
                         auto nextAttrPos = attrReader.tell() + subAttrLength;
                         
                         if (subAttrNameIndex < classFile->constant_pool.size()) {
                             auto subNameInfo = std::dynamic_pointer_cast<ConstantUtf8>(classFile->constant_pool[subAttrNameIndex]);
                             if (subNameInfo && subNameInfo->bytes == "LineNumberTable") {
                                 uint16_t lineNumberTableLength = attrReader.readU2();
                                 for (int j = 0; j < lineNumberTableLength; j++) {
                                     LineNumberTableEntry entry;
                                     entry.startPc = attrReader.readU2();
                                     entry.lineNumber = attrReader.readU2();
                                     lineNumberTable.push_back(entry);
                                 }
                             }
                         }
                         attrReader.seek(nextAttrPos);
                     }
                 // }
                 break;
            }
        }
    }
}

int StackFrame::getLineNumber(uint32_t pc) const {
    int line = -1;
    // Find the entry with the largest startPc that is <= pc
    for (const auto& entry : lineNumberTable) {
        if (entry.startPc <= pc) {
            // Since we don't know if it's sorted, we check if this entry is "closer" or "better"
            // Actually, usually we just want the one that starts most recently.
            // If the table is sorted, we can just iterate.
            // If not sorted, we need to find max(startPc) <= pc.
            
            // Simple approach: assume unsorted, find best match.
            // But wait, line numbers can jump around (loops).
            // But startPc mapping is unique for a given instruction range?
            // "The value of the start_pc item must indicate the index into the code array at which the code for a new line in the original source file begins."
            
            // So if we have entries (0, 10), (5, 11).
            // PC 6 -> Line 11.
            // PC 2 -> Line 10.
            
            // So we want the entry with startPc <= pc that has the MAX startPc.
            // If line variable is initialized to -1, we update it if we find a better match.
            // But we need to keep track of "best startPc".
            // Since we iterate, we can't easily compare with "current best".
            // Wait, we CAN.
        }
    }
    
    int bestStartPc = -1;
    for (const auto& entry : lineNumberTable) {
        if (entry.startPc <= pc) {
            if ((int)entry.startPc > bestStartPc) {
                bestStartPc = entry.startPc;
                line = entry.lineNumber;
            }
        }
    }
    return line;
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
        // 返回默认值 0/null
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
