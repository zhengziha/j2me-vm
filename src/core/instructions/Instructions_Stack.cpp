#include "../Interpreter.hpp"
#include "../Opcodes.hpp"
#include <iostream>

namespace j2me {
namespace core {

void Interpreter::initStack() {
    instructionTable[OP_POP] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            frame->pop();
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_POP2] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            JavaValue v = frame->pop();
            if (v.type != JavaValue::LONG && v.type != JavaValue::DOUBLE) {
                frame->pop();
            }
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DUP] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            JavaValue v1 = frame->pop();
            frame->push(v1);
            frame->push(v1);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DUP_X1] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            JavaValue v1 = frame->pop();
            JavaValue v2 = frame->pop();
            frame->push(v1);
            frame->push(v2);
            frame->push(v1);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DUP_X2] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            JavaValue v1 = frame->pop();
            JavaValue v2 = frame->pop();
            if (v2.type == JavaValue::LONG || v2.type == JavaValue::DOUBLE) {
                // Form 2: v1, v2(long) -> v1, v2, v1
                frame->push(v1);
                frame->push(v2);
                frame->push(v1);
            } else {
                // Form 1: v1, v2, v3 -> v1, v2, v3, v1
                JavaValue v3 = frame->pop();
                frame->push(v1);
                frame->push(v3);
                frame->push(v2);
                frame->push(v1);
            }
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DUP2] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            JavaValue v1 = frame->pop();
            if (v1.type == JavaValue::LONG || v1.type == JavaValue::DOUBLE) {
                frame->push(v1);
                frame->push(v1);
            } else {
                JavaValue v2 = frame->pop();
                frame->push(v2);
                frame->push(v1);
                frame->push(v2);
                frame->push(v1);
            }
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DUP2_X1] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            JavaValue v1 = frame->pop();
            if (v1.type == JavaValue::LONG || v1.type == JavaValue::DOUBLE) {
                // Form 2: v1(long), v2 -> v1, v2, v1
                JavaValue v2 = frame->pop();
                frame->push(v1);
                frame->push(v2);
                frame->push(v1);
            } else {
                // Form 1: v1, v2, v3 -> v2, v1, v3, v2, v1
                JavaValue v2 = frame->pop();
                JavaValue v3 = frame->pop();
                frame->push(v2);
                frame->push(v1);
                frame->push(v3);
                frame->push(v2);
                frame->push(v1);
            }
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DUP2_X2] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            JavaValue v1 = frame->pop();
            if (v1.type == JavaValue::LONG || v1.type == JavaValue::DOUBLE) {
                JavaValue v2 = frame->pop();
                if (v2.type == JavaValue::LONG || v2.type == JavaValue::DOUBLE) {
                     // Form 4: v1(long), v2(long) -> v1, v2, v1
                     frame->push(v1);
                     frame->push(v2);
                     frame->push(v1);
                } else {
                     // Form 2: v1(long), v2, v3 -> v1, v2, v3, v1
                     JavaValue v3 = frame->pop();
                     frame->push(v1);
                     frame->push(v3);
                     frame->push(v2);
                     frame->push(v1);
                }
            } else {
                JavaValue v2 = frame->pop();
                JavaValue v3 = frame->pop();
                if (v3.type == JavaValue::LONG || v3.type == JavaValue::DOUBLE) {
                    // Form 3: v1, v2, v3(long) -> v2, v1, v3, v2, v1
                    frame->push(v2);
                    frame->push(v1);
                    frame->push(v3);
                    frame->push(v2);
                    frame->push(v1);
                } else {
                    // Form 1: v1, v2, v3, v4 -> v2, v1, v4, v3, v2, v1
                    JavaValue v4 = frame->pop();
                    frame->push(v2);
                    frame->push(v1);
                    frame->push(v4);
                    frame->push(v3);
                    frame->push(v2);
                    frame->push(v1);
                }
            }
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_SWAP] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            JavaValue v1 = frame->pop();
            JavaValue v2 = frame->pop();
            frame->push(v1);
            frame->push(v2);
            break;
        } while(0);
        return true;
    };
}

}
}
