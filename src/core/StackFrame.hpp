#pragma once

#include <vector>
#include <cstdint>
#include <variant>
#include <string>
#include "ClassFile.hpp"

namespace j2me {
namespace core {

// Simple value type for stack and locals
// 用于操作数栈和局部变量表的简单数值类型
// In a real VM, this would be a more complex object model (JavaObject*)
// 在真实的虚拟机中，这会是一个更复杂的对象模型
struct JavaValue {
    enum Type {
        INT,        // 整型
        LONG,       // 长整型
        FLOAT,      // 浮点型
        DOUBLE,     // 双精度浮点型
        REFERENCE   // 对象引用
    } type; // 数据类型

    union {
        int32_t i;  // 32位整数
        int64_t l;  // 64位长整数
        float f;    // 32位浮点数
        double d;   // 64位双精度浮点数
        // For now, reference is just a raw pointer or ID. 
        // 目前引用只是一个原始指针，用于 Phase 1 的简化实现
        void* ref; 
    } val; // 存储的具体数值
    
    // Hack for HelloWorld string
    // 用于 HelloWorld 的临时字符串 hack
    std::string strVal; 
};

class StackFrame {
public:
    // 构造函数: 初始化方法的执行栈帧
    StackFrame(const MethodInfo& method, const std::shared_ptr<ClassFile>& classFile);

    // 操作数栈操作: 压入、弹出、查看栈顶
    void push(JavaValue value);
    JavaValue pop();
    JavaValue peek();
    
    // 调试辅助: 获取当前操作数栈的大小
    size_t size() const { return operandStack.size(); }

    // Exception Table Entry
    struct ExceptionTableEntry {
        uint16_t startPc;
        uint16_t endPc;
        uint16_t handlerPc;
        uint16_t catchType; // Index into constant pool
    };

    struct LineNumberTableEntry {
        uint16_t startPc;
        uint16_t lineNumber;
    };

    // 局部变量表操作: 设置、获取指定索引的局部变量
    void setLocal(uint16_t index, JavaValue value);
    JavaValue getLocal(uint16_t index);
    
    // Get line number for current PC
    int getLineNumber(uint32_t pc) const;

    // 操作数栈大小和判空
    size_t stackSize() const { return operandStack.size(); }
    bool isStackEmpty() const { return operandStack.empty(); }

    const MethodInfo& method;               // 当前执行的方法信息
    std::shared_ptr<ClassFile> classFile;   // 该方法所属的类文件
    uint32_t pc = 0;                        // 程序计数器 (Program Counter)，记录当前执行的字节码位置
    std::vector<uint8_t> code;              // 缓存的方法字节码数据
    std::vector<ExceptionTableEntry> exceptionTable; // 异常处理表
    std::vector<LineNumberTableEntry> lineNumberTable; // 行号表

private:
    std::vector<JavaValue> operandStack;    // 操作数栈 (LIFO)
    std::vector<JavaValue> localVariables;  // 局部变量表
};

} // namespace core
} // namespace j2me
