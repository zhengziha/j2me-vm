#pragma once

#include "StackFrame.hpp"
#include "../util/DataReader.hpp"
#include "../loader/JarLoader.hpp"
#include "RuntimeTypes.hpp"
#include "NativeRegistry.hpp"
#include "JavaThread.hpp"
#include <memory>
#include <map>
#include <optional>


namespace j2me {
namespace natives {
    void registerMediaNatives(j2me::core::NativeRegistry& registry);
}

namespace core {

class Interpreter {
public:
    Interpreter(j2me::loader::JarLoader& loader);
    
    // Execute instructions for a thread
    // 为指定线程执行一定数量的指令
    // instructions: Number of instructions to execute (quota) / 执行的指令配额
    // Returns: Number of instructions actually executed / 返回实际执行的指令数
    int execute(std::shared_ptr<JavaThread> thread, int instructions);

    // Resolve a class by name (loading it if necessary)
    // 根据名称解析类 (如果需要则加载)
    std::shared_ptr<JavaClass> resolveClass(const std::string& className);
    
    // Directly register a class (useful for .class files loaded directly)
    // 直接注册一个类 (用于直接加载的 .class 文件)
    void registerClass(const std::string& className, std::shared_ptr<JavaClass> cls);
    
    // Set the system library loader (rt.jar)
    // 设置系统库加载器 (rt.jar)
    void setLibraryLoader(std::shared_ptr<j2me::loader::JarLoader> loader) { libraryLoader = loader; }

private:
    j2me::loader::JarLoader& jarLoader; // Application loader / 应用加载器
    std::shared_ptr<j2me::loader::JarLoader> libraryLoader; // Library loader / 库加载器
    std::map<std::string, std::shared_ptr<JavaClass>> loadedClasses; // Loaded classes cache / 已加载类的缓存

    // Execute a single instruction
    // 执行单条指令
    bool executeInstruction(std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader);
    
    // Execute static initializer for a class
    // Returns true if initialization was triggered (and caller should rewind PC/retry)
    // 执行类的静态初始化器 (<clinit>)
    // 如果触发了初始化 (调用者需要回退 PC 并重试)，则返回 true
    bool initializeClass(std::shared_ptr<JavaThread> thread, std::shared_ptr<JavaClass> cls);
    
    // Validate that a string is actually a class name, not a descriptor
    // 验证字符串是否为有效的类名 (不是描述符)
    bool isValidClassName(const std::string& name);

    // Handle exception throwing
    // 处理异常抛出
    // Returns true if exception was handled (caught in current or caller frame), false otherwise
    // 如果异常被处理 (在当前或调用者栈帧中捕获)，返回 true；否则返回 false
    bool handleException(std::shared_ptr<JavaThread> thread, JavaObject* exception);

    // Make friends for native access
    friend class j2me::core::NativeRegistry;
    friend void j2me::natives::registerMediaNatives(j2me::core::NativeRegistry& registry);

    // Instruction handler function type
    // 指令处理函数类型
    using InstructionHandler = std::function<bool(std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame>, util::DataReader&, uint8_t)>;
    
    // Dispatch table for instructions (0x00 - 0xFF)
    // 指令分发表 (0x00 - 0xFF)
    std::vector<InstructionHandler> instructionTable;
    
    // Initialize the instruction table
    // 初始化指令表
    void initInstructionTable();

    // Instruction group initializers
    // 指令组初始化函数
    void initConstants();    // 常量加载指令
    void initLoads();        // 加载指令 (局部变量 -> 栈)
    void initStores();       // 存储指令 (栈 -> 局部变量)
    void initStack();        // 栈操作指令 (DUP, POP 等)
    void initMath();         // 算术指令
    void initConversions();  // 类型转换指令
    void initComparisons();  // 比较指令
    void initControl();      // 控制流指令 (跳转, 返回)
    void initReferences();   // 对象引用指令 (NEW, INVOKE 等)
    void initExtended();     // 扩展指令 (WIDE 等)
};

} // namespace core
} // namespace j2me
