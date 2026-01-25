#include "Interpreter.hpp"
#include "Opcodes.hpp"
#include "ClassParser.hpp"
#include "HeapManager.hpp"
#include "NativeRegistry.hpp"
#include "Logger.hpp"
#include "EventLoop.hpp"
#include <iostream>
#include <sstream>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <functional>
#include <vector>

namespace j2me {
namespace core {

Interpreter::Interpreter(j2me::loader::JarLoader& loader) : jarLoader(loader) { initInstructionTable(); }

// resolveClass is implemented in Interpreter_ClassLoader.cpp

int Interpreter::execute(std::shared_ptr<JavaThread> thread, int instructions) {
    if (!thread || thread->isFinished()) return 0;
    if (thread->state != JavaThread::RUNNABLE) return 0;

    int executed = 0;
    while (executed < instructions && !thread->isFinished()) {
        if (thread->state != JavaThread::RUNNABLE) break;

        if (EventLoop::getInstance().shouldExit()) {
             // 处理退出请求
             // Handle exit
             return executed;
        }

        auto frame = thread->currentFrame();
        if (!frame) break;

        // Code is cached in frame
        if (frame->code.empty()) {
            // 如果代码为空 (例如 native 或 abstract 方法)，弹出栈帧
            // Should be native or abstract, but if it's on stack, pop it?
            thread->popFrame();
            continue;
        }

        util::DataReader codeReader(frame->code);
        codeReader.seek(frame->pc);
        
        uint8_t opcode = 0;
        if (frame->pc < frame->code.size()) opcode = frame->code[frame->pc];
        
        try {
            // 执行单条指令
            // executeInstruction now modifies thread state directly (pushes/pops frames)
            // It returns false if we should stop execution immediately (error), otherwise true
            bool continueExec = executeInstruction(thread, frame, codeReader);
            
            // 更新 PC (程序计数器)
            // Update PC
            frame->pc = codeReader.tell();
            executed++;
            
            if (!continueExec) {
                break;
            }
        } catch (const std::exception& e) {
            std::string msg = e.what() ? std::string(e.what()) : std::string();
            std::string exClass = "java/lang/RuntimeException";
            if (msg.find("NullPointerException") != std::string::npos) exClass = "java/lang/NullPointerException";
            else if (msg.find("ArrayIndexOutOfBoundsException") != std::string::npos) exClass = "java/lang/ArrayIndexOutOfBoundsException";
            else if (msg.find("StringIndexOutOfBoundsException") != std::string::npos) exClass = "java/lang/StringIndexOutOfBoundsException";
            else if (msg.find("ArithmeticException") != std::string::npos) exClass = "java/lang/ArithmeticException";
            else if (msg.find("ClassCastException") != std::string::npos) exClass = "java/lang/ClassCastException";
            else if (msg.find("NegativeArraySizeException") != std::string::npos) exClass = "java/lang/NegativeArraySizeException";

            auto exCls = resolveClass(exClass);
            if (!exCls) {
                LOG_ERROR("Runtime Exception: " + msg);
                thread->frames.clear();
                thread->state = JavaThread::TERMINATED;
                break;
            }

            JavaObject* exObj = HeapManager::getInstance().allocate(exCls);
            bool handled = handleException(thread, exObj);
            if (!handled) {
                LOG_ERROR("VM terminated due to uncaught exception: " + exClass);
                thread->frames.clear();
                thread->state = JavaThread::TERMINATED;
                break;
            }
        }
    }
    return executed;
}

bool Interpreter::initializeClass(std::shared_ptr<JavaThread> thread, std::shared_ptr<JavaClass> cls) {
    if (cls->initialized) {
        return false;
    }
    
    // 防止循环初始化
    // Prevent circular initialization
    if (cls->initializing) {
        return false; 
    }
    
    cls->initializing = true;
    LOG_DEBUG("[Interpreter] Initializing class: " + cls->name);
    
    bool pushed = false;

    // 首先初始化父类 (如果存在)
    // First, initialize superclass if exists
    if (cls->superClass) {
        if (initializeClass(thread, cls->superClass)) {
            pushed = true;
        }
    }
    
    // 查找并执行 <clinit> 方法 (类初始化器)
    // Find and execute <clinit> method
    for (const auto& method : cls->rawFile->methods) {
        auto name = std::dynamic_pointer_cast<ConstantUtf8>(cls->rawFile->constant_pool[method.name_index]);
        if (name && name->bytes == "<clinit>") {
            LOG_DEBUG("[Interpreter] Pushing <clinit> for: " + cls->name);
            auto frame = std::make_shared<StackFrame>(method, cls->rawFile);
            thread->pushFrame(frame);
            pushed = true;
            break;
        }
    }
    
    cls->initialized = true;
    cls->initializing = false; 
    
    return pushed;
}

bool Interpreter::executeInstruction(std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader) {
    static uint64_t instructionCount = 0;
    instructionCount++;

    uint8_t opcode = codeReader.readU1();
    
    // Debug logging for crash tracking
    // std::cout << "Exec: " << std::hex << (int)opcode << " PC: " << (codeReader.tell()-1) << std::dec << std::endl;

    if (instructionTable[opcode]) {
        return instructionTable[opcode](thread, frame, codeReader, opcode);
    } else {
        std::cerr << "Unknown Opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
        return true;
    }
    return true; // Continue execution
}

// isValidClassName is implemented in Interpreter_ClassLoader.cpp

bool Interpreter::handleException(std::shared_ptr<JavaThread> thread, JavaObject* exception) {
    if (!thread || !exception) return false;
    
    // First, verify if we can handle this exception
    // We do a dry run of stack unwinding to find a handler without modifying the thread state yet.
    // However, our logic modifies thread state (popFrame).
    // So we should capture the stack trace first, but only print it if UNCAUGHT.
    
    std::string exName = "Unknown";
    if (exception->cls) exName = exception->cls->name;

    std::stringstream stackTraceBuffer;
    stackTraceBuffer << "Exception thrown: " << exName << std::endl;
    stackTraceBuffer << "Stack Trace:" << std::endl;
    
    for (auto it = thread->frames.rbegin(); it != thread->frames.rend(); ++it) {
        auto f = *it;
        std::string cName = "Unknown";
        std::string mName = "Unknown";
        if (f->classFile) {
            auto cls = std::dynamic_pointer_cast<ConstantClass>(f->classFile->constant_pool[f->classFile->this_class]);
            if (cls) {
                auto utf8 = std::dynamic_pointer_cast<ConstantUtf8>(f->classFile->constant_pool[cls->name_index]);
                if (utf8) cName = utf8->bytes;
            }
            if (f->method.name_index < f->classFile->constant_pool.size()) {
                auto mUtf8 = std::dynamic_pointer_cast<ConstantUtf8>(f->classFile->constant_pool[f->method.name_index]);
                if (mUtf8) mName = mUtf8->bytes;
            }
        }
        int lineNum = f->getLineNumber(f->pc);
        stackTraceBuffer << "  at " << cName << "." << mName << " (PC: " << f->pc << ", Line: " << lineNum << ")" << std::endl;
    }

    bool isTopFrame = true;
    while (true) {
        auto frame = thread->currentFrame();
        if (!frame) {
             // Uncaught! Print the buffered stack trace and return false
             std::cerr << stackTraceBuffer.str();
             return false; 
        }

        int searchPc = frame->pc;
        if (!isTopFrame) {
            // For caller frames, pc is the return address (next instruction).
            // We need the pc of the invoke instruction, which is definitely before current pc.
            // Using pc - 1 is sufficient to find the correct range.
            if (searchPc > 0) searchPc--;
        }

        // Search for exception handler in current frame
        int handlerPc = -1;
        
        for (const auto& entry : frame->exceptionTable) {
            // Check if PC is within range [startPc, endPc)
            
            if (searchPc >= entry.startPc && searchPc < entry.endPc) {
                // Check catch type
                if (entry.catchType == 0) {
                    // catch_type 0 means finally (matches any exception)
                    handlerPc = entry.handlerPc;
                    break;
                } else {
                    // Resolve catch type class
                    auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[entry.catchType]);
                    if (classRef) {
                        auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
                        if (className) {
                            // Check if exception is instance of catch type
                            // We need to resolve the catch class first
                            auto catchClass = resolveClass(className->bytes);
                            if (catchClass) {
                                // Manual instanceof check
                                bool isInstance = false;
                                if (exception->cls) {
                                     std::function<bool(std::shared_ptr<JavaClass>, const std::string&)> isAssignable = 
                                         [&](std::shared_ptr<JavaClass> sub, const std::string& target) -> bool {
                                             if (!sub) return false;
                                             if (sub->name == target) return true;
                                             if (isAssignable(sub->superClass, target)) return true;
                                             for (auto& iface : sub->interfaces) {
                                                  if (isAssignable(iface, target)) return true;
                                             }
                                             return false;
                                         };
                    
                                     if (isAssignable(exception->cls, catchClass->name)) {
                                         isInstance = true;
                                     }
                                }
                                
                                if (isInstance) {
                                    handlerPc = entry.handlerPc;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (handlerPc != -1) {
            // Found handler
            
            // Only log at debug level since it's caught
            // LOG_DEBUG("Exception handled: " + exName + " at PC " + std::to_string(handlerPc));
            
            // Clear operand stack
            while (!frame->isStackEmpty()) {
                frame->pop();
            }
            // Push exception object
            JavaValue val;
            val.type = JavaValue::REFERENCE;
            val.val.ref = exception;
            frame->push(val);
            
            // Jump to handler
            frame->pc = handlerPc;
            return true;
        }

        // No handler in current frame, pop and continue in caller
        thread->popFrame();
        isTopFrame = false;
        if (thread->isFinished()) {
             // Uncaught! Print the buffered stack trace
             std::cerr << stackTraceBuffer.str();
             return false; 
        }
    }
}

void Interpreter::initInstructionTable() {
    instructionTable.resize(256);
    // Default handler
    for(int i=0; i<256; i++) instructionTable[i] = [](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame>, util::DataReader&, uint8_t opcode) -> bool { 
        std::cerr << "Unknown Opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
        return true;
    };

    initConstants();
    initLoads();
    initStores();
    initStack();
    initMath();
    initConversions();
    initComparisons();
    initControl();
    initReferences();
    initExtended();
}

} // namespace core
} // namespace j2me
