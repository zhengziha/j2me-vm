#pragma once

#include <vector>
#include <cstdint>
#include <variant>
#include <string>
#include "ClassFile.hpp"

namespace j2me {
namespace core {

// Simple value type for stack and locals
// In a real VM, this would be a more complex object model (JavaObject*)
// For now, we support int and string (for Hello World)
struct JavaValue {
    enum Type {
        INT,
        LONG,
        FLOAT,
        DOUBLE,
        REFERENCE
    } type;

    union {
        int32_t i;
        int64_t l;
        float f;
        double d;
        // For now, reference is just a raw pointer or ID. 
        // We'll use a void* or int for simplicity in Phase 1.
        void* ref; 
    } val;
    
    // Hack for HelloWorld string
    std::string strVal; 
};

class StackFrame {
public:
    StackFrame(const MethodInfo& method, const std::shared_ptr<ClassFile>& classFile);

    void push(JavaValue value);
    JavaValue pop();
    JavaValue peek();
    
    // Debug helper
    size_t size() const { return operandStack.size(); }

    void setLocal(uint16_t index, JavaValue value);
    JavaValue getLocal(uint16_t index);
    
    size_t stackSize() const { return operandStack.size(); }
    bool isStackEmpty() const { return operandStack.empty(); }

    const MethodInfo& method;
    const std::shared_ptr<ClassFile>& classFile;
    uint32_t pc = 0; // Program Counter

private:
    std::vector<JavaValue> operandStack;
    std::vector<JavaValue> localVariables;
};

} // namespace core
} // namespace j2me
