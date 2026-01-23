#include "../Interpreter.hpp"
#include "../Opcodes.hpp"
#include <iostream>

namespace j2me {
namespace core {

void Interpreter::initControl() {
    instructionTable[OP_TABLESWITCH] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            size_t pc = codeReader.tell() - 1;
            size_t padding = (4 - (pc + 1) % 4) % 4;
            for (size_t i = 0; i < padding; i++) codeReader.readU1();
            
            int32_t defaultOffset = (int32_t)codeReader.readU4();
            int32_t low = (int32_t)codeReader.readU4();
            int32_t high = (int32_t)codeReader.readU4();
            
            int32_t index = frame->pop().val.i;
            
            if (index < low || index > high) {
                codeReader.seek(pc + defaultOffset);
            } else {
                size_t jumpTableStart = codeReader.tell();
                size_t entryOffset = (index - low) * 4;
                codeReader.seek(jumpTableStart + entryOffset);
                int32_t offset = (int32_t)codeReader.readU4();
                codeReader.seek(pc + offset);
            }
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LOOKUPSWITCH] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            size_t pc = codeReader.tell() - 1;
            size_t padding = (4 - (pc + 1) % 4) % 4;
            
            for (size_t i = 0; i < padding; i++) codeReader.readU1();
            
            if (codeReader.tell() + 8 > frame->code.size()) {
                std::cerr << "[ERROR] LOOKUPSWITCH: Insufficient bytes for default/npairs at PC=" << pc << std::endl;
                return false; 
            }

            int32_t defaultOffset = (int32_t)codeReader.readU4();
            int32_t npairs = (int32_t)codeReader.readU4();
            
            if (npairs < 0 || npairs > 65535) { // Sanity check
                 std::cerr << "[ERROR] LOOKUPSWITCH: Invalid npairs " << npairs << " at PC=" << pc << std::endl;
                 return false;
            }

            if (codeReader.tell() + (size_t)npairs * 8 > frame->code.size()) {
                 std::cerr << "[ERROR] LOOKUPSWITCH: Insufficient bytes for pairs at PC=" << pc << std::endl;
                 return false;
            }

            int32_t key = frame->pop().val.i;
            int32_t targetOffset = defaultOffset;
            
            for (int i = 0; i < npairs; i++) {
                int32_t match = (int32_t)codeReader.readU4();
                int32_t offset = (int32_t)codeReader.readU4();
                if (match == key) {
                    targetOffset = offset;
                    break;
                }
            }
            codeReader.seek(pc + targetOffset);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_GOTO] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             codeReader.seek(codeReader.tell() + offset - 3); 
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_GOTO_W] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int32_t offset = (int32_t)codeReader.readU4();
             codeReader.seek(codeReader.tell() + offset - 5); 
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_RETURN] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            thread->popFrame();
            return true; // Continue execution (next frame)
        } while(0);
        return true;
    };

    instructionTable[OP_IRETURN] = instructionTable[OP_ARETURN] = instructionTable[OP_LRETURN] = instructionTable[OP_FRETURN] = instructionTable[OP_DRETURN] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            JavaValue retVal = frame->pop();
            thread->popFrame();
            
            // Push return value to caller's stack
            if (!thread->isFinished()) {
                auto callerFrame = thread->currentFrame();
                if (callerFrame) {
                    callerFrame->push(retVal);
                }
            }
            return true; // Continue execution
        } while(0);
        return true;
    };
}

}
}
