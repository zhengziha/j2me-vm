#include "Interpreter.hpp"
#include "Opcodes.hpp"
#include "ClassParser.hpp"
#include "HeapManager.hpp"
#include "NativeRegistry.hpp"
#include "Logger.hpp"
#include "EventLoop.hpp"
#include <iostream>
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
             // Handle exit
             return executed;
        }

        auto frame = thread->currentFrame();
        if (!frame) break;

        // Code is cached in frame
        if (frame->code.empty()) {
            // Should be native or abstract, but if it's on stack, pop it?
            thread->popFrame();
            continue;
        }

        util::DataReader codeReader(frame->code);
        codeReader.seek(frame->pc);
        
        uint8_t opcode = 0;
        if (frame->pc < frame->code.size()) opcode = frame->code[frame->pc];
        
        try {
            // executeInstruction now modifies thread state directly (pushes/pops frames)
            // It returns false if we should stop execution immediately (error), otherwise true
            bool continueExec = executeInstruction(thread, frame, codeReader);
            
            // Update PC
            frame->pc = codeReader.tell();
            executed++;
            
            if (!continueExec) {
                break;
            }
        } catch (const std::exception& e) {
             LOG_ERROR("Runtime Exception: " + std::string(e.what()));
             
             if (frame && frame->classFile) {
                 // Get Class Name
                 std::string className = "Unknown";
                 if (frame->classFile->constant_pool.size() > frame->classFile->this_class) {
                     auto classInfo = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[frame->classFile->this_class]);
                     if (classInfo) {
                         auto utf8 = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classInfo->name_index]);
                         if (utf8) className = utf8->bytes;
                     }
                 }
                 
                 // Get Method Name
                 std::string methodName = "Unknown";
                 if (frame->method.name_index < frame->classFile->constant_pool.size()) {
                     auto utf8 = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[frame->method.name_index]);
                     if (utf8) methodName = utf8->bytes;
                 }
                 
                 LOG_ERROR("Exception at " + className + "." + methodName + ", PC: " + std::to_string(frame->pc));
                 
                 if (frame->pc < frame->code.size()) {
                     uint8_t op = frame->code[frame->pc];
                      std::cerr << "Opcode at PC " << frame->pc << ": 0x" << std::hex << (int)op << std::dec << std::endl;
                  }
                  
                  // Dump Operand Stack
                  std::cerr << "Operand Stack (Top to Bottom):" << std::endl;
                  if (frame->isStackEmpty()) {
                      std::cerr << "  <empty>" << std::endl;
                  } else {
                      // We can't access private members directly unless we are friends or have getters.
                      // StackFrame has no public iterator.
                      // But we can peek.
                      // Actually, let's just print the size for now, or add a debug method to StackFrame.
                      std::cerr << "  Size: " << frame->stackSize() << std::endl;
                      if (frame->stackSize() > 0) {
                           JavaValue top = frame->peek();
                           std::cerr << "  Top Type: " << top.type;
                           if (top.type == JavaValue::REFERENCE) {
                               std::cerr << " Val: " << top.val.ref;
                           } else {
                               std::cerr << " Val: " << top.val.i << " (hex " << std::hex << top.val.i << std::dec << ")";
                           }
                           std::cerr << std::endl;
                       }
                  }

                  // Print Stack Trace
                 std::cerr << "Stack Trace:" << std::endl;
                 auto currentThread = thread;
                 while (currentThread) {
                     auto currentFrame = currentThread->currentFrame();
                     if (!currentFrame) break;
                     
                     std::string cName = "Unknown";
                     std::string mName = "Unknown";
                     if (currentFrame->classFile) {
                         auto cls = std::dynamic_pointer_cast<ConstantClass>(currentFrame->classFile->constant_pool[currentFrame->classFile->this_class]);
                         auto utf8 = std::dynamic_pointer_cast<ConstantUtf8>(currentFrame->classFile->constant_pool[cls->name_index]);
                         cName = utf8->bytes;
                         
                         if (currentFrame->method.name_index < currentFrame->classFile->constant_pool.size()) {
                             auto mUtf8 = std::dynamic_pointer_cast<ConstantUtf8>(currentFrame->classFile->constant_pool[currentFrame->method.name_index]);
                             mName = mUtf8->bytes;
                         }
                     }
                     std::cerr << "  at " << cName << "." << mName << " (PC: " << currentFrame->pc << ")" << std::endl;
                     
                     currentThread->popFrame(); // Unwind for trace (destructive but we are crashing anyway)
                 }
             }
             
             // Do not rethrow, just exit the thread gracefully to prevent process termination
             LOG_ERROR("Thread terminated due to uncaught exception.");
             break; 
        }
    }
    return executed;
}

bool Interpreter::initializeClass(std::shared_ptr<JavaThread> thread, std::shared_ptr<JavaClass> cls) {
    if (cls->initialized) {
        return false;
    }
    
    // Prevent circular initialization
    if (cls->initializing) {
        return false; 
    }
    
    cls->initializing = true;
    LOG_DEBUG("[Interpreter] Initializing class: " + cls->name);
    
    bool pushed = false;

    // First, initialize superclass if exists
    if (cls->superClass) {
        if (initializeClass(thread, cls->superClass)) {
            pushed = true;
        }
    }
    
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
