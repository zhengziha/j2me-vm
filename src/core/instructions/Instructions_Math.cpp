#include "../Interpreter.hpp"
#include "../Opcodes.hpp"
#include <iostream>
#include <cmath>

namespace j2me {
namespace core {

void Interpreter::initMath() {
    instructionTable[OP_IADD] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 + v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LADD] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 + v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_FADD] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            float v2 = frame->pop().val.f;
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = v1 + v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DADD] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            double v2 = frame->pop().val.d;
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = v1 + v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_ISUB] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 - v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LSUB] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 - v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_FSUB] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            float v2 = frame->pop().val.f;
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = v1 - v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DSUB] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            double v2 = frame->pop().val.d;
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = v1 - v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_IMUL] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 * v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LMUL] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 * v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_FMUL] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            float v2 = frame->pop().val.f;
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = v1 * v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DMUL] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            double v2 = frame->pop().val.d;
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = v1 * v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_IDIV] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            if (v2 == 0) throw std::runtime_error("ArithmeticException: / by zero");
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 / v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LDIV] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            if (v2 == 0) throw std::runtime_error("ArithmeticException: / by zero");
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 / v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_FDIV] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            float v2 = frame->pop().val.f;
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = v1 / v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DDIV] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            double v2 = frame->pop().val.d;
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = v1 / v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_IREM] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            if (v2 == 0) throw std::runtime_error("ArithmeticException: / by zero");
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 % v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LREM] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            if (v2 == 0) throw std::runtime_error("ArithmeticException: / by zero");
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 % v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_FREM] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            float v2 = frame->pop().val.f;
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = std::fmod(v1, v2);
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DREM] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            double v2 = frame->pop().val.d;
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = std::fmod(v1, v2);
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_INEG] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = -v1;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LNEG] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = -v1;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_FNEG] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = -v1;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DNEG] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = -v1;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_ISHL] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 << (v2 & 0x1F);
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LSHL] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v2 = frame->pop().val.i;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 << (v2 & 0x3F);
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_ISHR] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 >> (v2 & 0x1F);
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LSHR] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v2 = frame->pop().val.i;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 >> (v2 & 0x3F);
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_IUSHR] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = (uint32_t)v1 >> (v2 & 0x1F);
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LUSHR] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v2 = frame->pop().val.i;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = (uint64_t)v1 >> (v2 & 0x3F);
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_IAND] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 & v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LAND] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 & v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_IOR] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 | v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LOR] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 | v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_IXOR] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 ^ v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LXOR] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 ^ v2;
            frame->push(res);
            break;
        } while(0);
        return true;
    };
    
    instructionTable[OP_IINC] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            uint8_t index = codeReader.readU1();
            int8_t constVal = (int8_t)codeReader.readU1();
            JavaValue val = frame->getLocal(index);
            val.val.i += constVal;
            frame->setLocal(index, val);
            break;
        } while(0);
        return true;
    };
}

}
}
