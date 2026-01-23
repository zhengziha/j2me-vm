#include "../Interpreter.hpp"
#include "../Opcodes.hpp"
#include <iostream>

namespace j2me {
namespace core {

void Interpreter::initConversions() {
    instructionTable[OP_I2L] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = (int64_t)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_I2F] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = (float)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_I2D] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = (double)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_L2I] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int64_t v = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::INT; res.val.i = (int32_t)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_L2F] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int64_t v = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = (float)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_L2D] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int64_t v = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = (double)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_F2I] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            float v = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::INT; res.val.i = (int32_t)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_F2L] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            float v = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = (int64_t)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_F2D] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            float v = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = (double)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_D2I] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            double v = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::INT; res.val.i = (int32_t)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_D2L] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            double v = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = (int64_t)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_D2F] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            double v = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = (float)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_I2B] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = (int8_t)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_I2C] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = (uint16_t)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_I2S] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = (int16_t)v;
            frame->push(res);
            break;
        } while(0);
        return true;
    };
}

}
}
