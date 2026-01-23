#include "../Interpreter.hpp"
#include "../Opcodes.hpp"
#include "../HeapManager.hpp"
#include <iostream>
#include <cstring>

namespace j2me {
namespace core {

void Interpreter::initStores() {
    instructionTable[OP_ISTORE] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        uint8_t idx = codeReader.readU1();
        frame->setLocal(idx, frame->pop());
        return true;
    };
    instructionTable[OP_LSTORE] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        uint8_t idx = codeReader.readU1();
        frame->setLocal(idx, frame->pop());
        return true;
    };
    instructionTable[OP_FSTORE] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        uint8_t idx = codeReader.readU1();
        frame->setLocal(idx, frame->pop());
        return true;
    };
    instructionTable[OP_DSTORE] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        uint8_t idx = codeReader.readU1();
        frame->setLocal(idx, frame->pop());
        return true;
    };
    instructionTable[OP_ASTORE] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        uint8_t idx = codeReader.readU1();
        frame->setLocal(idx, frame->pop());
        return true;
    };
    instructionTable[OP_ISTORE_0] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(0, frame->pop());
        return true;
    };
    instructionTable[OP_LSTORE_0] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(0, frame->pop());
        return true;
    };
    instructionTable[OP_FSTORE_0] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(0, frame->pop());
        return true;
    };
    instructionTable[OP_DSTORE_0] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(0, frame->pop());
        return true;
    };
    instructionTable[OP_ASTORE_0] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(0, frame->pop());
        return true;
    };
    instructionTable[OP_ISTORE_1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(1, frame->pop());
        return true;
    };
    instructionTable[OP_LSTORE_1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(1, frame->pop());
        return true;
    };
    instructionTable[OP_FSTORE_1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(1, frame->pop());
        return true;
    };
    instructionTable[OP_DSTORE_1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(1, frame->pop());
        return true;
    };
    instructionTable[OP_ASTORE_1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(1, frame->pop());
        return true;
    };
    instructionTable[OP_ISTORE_2] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(2, frame->pop());
        return true;
    };
    instructionTable[OP_LSTORE_2] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(2, frame->pop());
        return true;
    };
    instructionTable[OP_FSTORE_2] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(2, frame->pop());
        return true;
    };
    instructionTable[OP_DSTORE_2] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(2, frame->pop());
        return true;
    };
    instructionTable[OP_ASTORE_2] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(2, frame->pop());
        return true;
    };
    instructionTable[OP_ISTORE_3] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(3, frame->pop());
        return true;
    };
    instructionTable[OP_LSTORE_3] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(3, frame->pop());
        return true;
    };
    instructionTable[OP_FSTORE_3] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(3, frame->pop());
        return true;
    };
    instructionTable[OP_DSTORE_3] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(3, frame->pop());
        return true;
    };
    instructionTable[OP_ASTORE_3] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t) -> bool {
        frame->setLocal(3, frame->pop());
        return true;
    };

    // Array Stores
    instructionTable[OP_IASTORE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t val = frame->pop().val.i;
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            arr->fields[index] = val;
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LASTORE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int64_t val = frame->pop().val.l;
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            arr->fields[index] = val;
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_FASTORE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            float val = frame->pop().val.f;
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            int32_t bits;
            memcpy(&bits, &val, sizeof(float));
            arr->fields[index] = (int64_t)bits;
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_DASTORE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            double val = frame->pop().val.d;
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            int64_t bits;
            memcpy(&bits, &val, sizeof(double));
            arr->fields[index] = bits;
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_AASTORE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            JavaValue val = frame->pop();
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            // Should check array store exception (type mismatch) here, but skipping for now
            arr->fields[index] = (int64_t)val.val.ref;
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_BASTORE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t val = frame->pop().val.i;
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            arr->fields[index] = (int8_t)val;
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_CASTORE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t val = frame->pop().val.i;
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            arr->fields[index] = (uint16_t)val;
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_SASTORE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int32_t val = frame->pop().val.i;
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            arr->fields[index] = (int16_t)val;
            break;
        } while(0);
        return true;
    };
}

}
}
