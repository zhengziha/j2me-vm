#include "../Interpreter.hpp"
#include "../Opcodes.hpp"
#include "../HeapManager.hpp"
#include <iostream>
#include <cstring>

namespace j2me {
namespace core {

void Interpreter::initLoads() {
    instructionTable[OP_ILOAD] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        uint8_t idx = codeReader.readU1();
        frame->push(frame->getLocal(idx));
        return true;
    };
    instructionTable[OP_LLOAD] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        uint8_t idx = codeReader.readU1();
        frame->push(frame->getLocal(idx));
        return true;
    };
    instructionTable[OP_FLOAD] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        uint8_t idx = codeReader.readU1();
        frame->push(frame->getLocal(idx));
        return true;
    };
    instructionTable[OP_DLOAD] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        uint8_t idx = codeReader.readU1();
        frame->push(frame->getLocal(idx));
        return true;
    };
    instructionTable[OP_ALOAD] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        uint8_t idx = codeReader.readU1();
        frame->push(frame->getLocal(idx));
        return true;
    };
    instructionTable[OP_ILOAD_0] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(0));
        return true;
    };
    instructionTable[OP_LLOAD_0] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(0));
        return true;
    };
    instructionTable[OP_FLOAD_0] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(0));
        return true;
    };
    instructionTable[OP_DLOAD_0] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(0));
        return true;
    };
    instructionTable[OP_ALOAD_0] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(0));
        return true;
    };
    instructionTable[OP_ILOAD_1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(1));
        return true;
    };
    instructionTable[OP_LLOAD_1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(1));
        return true;
    };
    instructionTable[OP_FLOAD_1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(1));
        return true;
    };
    instructionTable[OP_DLOAD_1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(1));
        return true;
    };
    instructionTable[OP_ALOAD_1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(1));
        return true;
    };
    instructionTable[OP_ILOAD_2] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(2));
        return true;
    };
    instructionTable[OP_LLOAD_2] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(2));
        return true;
    };
    instructionTable[OP_FLOAD_2] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(2));
        return true;
    };
    instructionTable[OP_DLOAD_2] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(2));
        return true;
    };
    instructionTable[OP_ALOAD_2] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(2));
        return true;
    };
    instructionTable[OP_ILOAD_3] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(3));
        return true;
    };
    instructionTable[OP_LLOAD_3] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(3));
        return true;
    };
    instructionTable[OP_FLOAD_3] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(3));
        return true;
    };
    instructionTable[OP_DLOAD_3] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(3));
        return true;
    };
    instructionTable[OP_ALOAD_3] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->push(frame->getLocal(3));
        return true;
    };

    // Array Loads
    instructionTable[OP_IALOAD] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            JavaValue val;
            val.type = JavaValue::INT;
            val.val.i = (int32_t)arr->fields[index];
            frame->push(val);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LALOAD] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            JavaValue val;
            val.type = JavaValue::LONG;
            val.val.l = arr->fields[index];
            frame->push(val);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_FALOAD] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            JavaValue val;
            val.type = JavaValue::FLOAT;
            int32_t bits = (int32_t)arr->fields[index];
            memcpy(&val.val.f, &bits, sizeof(float));
            frame->push(val);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DALOAD] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            JavaValue val;
            val.type = JavaValue::DOUBLE;
            int64_t bits = arr->fields[index];
            memcpy(&val.val.d, &bits, sizeof(double));
            frame->push(val);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_AALOAD] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            JavaValue val;
            val.type = JavaValue::REFERENCE;
            val.val.ref = (void*)arr->fields[index];
            frame->push(val);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_BALOAD] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            JavaValue val;
            val.type = JavaValue::INT;
            val.val.i = (int8_t)arr->fields[index]; // Sign extend for byte
            frame->push(val);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_CALOAD] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            JavaValue val;
            val.type = JavaValue::INT;
            val.val.i = (uint16_t)arr->fields[index]; // Zero extend for char
            frame->push(val);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_SALOAD] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            JavaValue val;
            val.type = JavaValue::INT;
            val.val.i = (int16_t)arr->fields[index]; // Sign extend for short
            frame->push(val);
            break;
        } while(0);
        return true;
    };
}

}
}
