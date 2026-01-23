#include "../Interpreter.hpp"
#include "../Opcodes.hpp"
#include <iostream>
#include <cmath>

namespace j2me {
namespace core {

void Interpreter::initComparisons() {
    instructionTable[OP_LCMP] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::INT;
            if (v1 > v2) res.val.i = 1;
            else if (v1 < v2) res.val.i = -1;
            else res.val.i = 0;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_FCMPL] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            float v2 = frame->pop().val.f;
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::INT;
            if (std::isnan(v1) || std::isnan(v2)) res.val.i = -1;
            else if (v1 > v2) res.val.i = 1;
            else if (v1 < v2) res.val.i = -1;
            else res.val.i = 0;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_FCMPG] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            float v2 = frame->pop().val.f;
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::INT;
            if (std::isnan(v1) || std::isnan(v2)) res.val.i = 1;
            else if (v1 > v2) res.val.i = 1;
            else if (v1 < v2) res.val.i = -1;
            else res.val.i = 0;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DCMPL] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            double v2 = frame->pop().val.d;
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::INT;
            if (std::isnan(v1) || std::isnan(v2)) res.val.i = -1;
            else if (v1 > v2) res.val.i = 1;
            else if (v1 < v2) res.val.i = -1;
            else res.val.i = 0;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DCMPG] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            double v2 = frame->pop().val.d;
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::INT;
            if (std::isnan(v1) || std::isnan(v2)) res.val.i = 1;
            else if (v1 > v2) res.val.i = 1;
            else if (v1 < v2) res.val.i = -1;
            else res.val.i = 0;
            frame->push(res);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_IFEQ] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val = frame->pop().val.i;
             if (val == 0) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_IFNE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val = frame->pop().val.i;
             if (val != 0) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_IFLT] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val = frame->pop().val.i;
             if (val < 0) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_IFGE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val = frame->pop().val.i;
             if (val >= 0) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_IFGT] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val = frame->pop().val.i;
             if (val > 0) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_IFLE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val = frame->pop().val.i;
             if (val <= 0) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_IF_ICMPEQ] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val2 = frame->pop().val.i;
             int32_t val1 = frame->pop().val.i;
             if (val1 == val2) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_IF_ICMPNE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val2 = frame->pop().val.i;
             int32_t val1 = frame->pop().val.i;
             if (val1 != val2) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_IF_ICMPLT] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val2 = frame->pop().val.i;
             int32_t val1 = frame->pop().val.i;
             
             if (val1 < val2) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_IF_ICMPGE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val2 = frame->pop().val.i;
             int32_t val1 = frame->pop().val.i;

             if (val1 >= val2) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_IF_ICMPGT] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val2 = frame->pop().val.i;
             int32_t val1 = frame->pop().val.i;

             if (val1 > val2) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_IF_ICMPLE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val2 = frame->pop().val.i;
             int32_t val1 = frame->pop().val.i;

             if (val1 <= val2) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_IF_ACMPEQ] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             JavaValue val2 = frame->pop();
             JavaValue val1 = frame->pop();
             if (val1.val.ref == val2.val.ref) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_IF_ACMPNE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             JavaValue val2 = frame->pop();
             JavaValue val1 = frame->pop();
             if (val1.val.ref != val2.val.ref) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };
}

}
}
