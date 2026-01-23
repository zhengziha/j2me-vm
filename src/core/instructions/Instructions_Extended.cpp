#include "../Interpreter.hpp"
#include "../Opcodes.hpp"
#include "../HeapManager.hpp"
#include <iostream>
#include <functional>
#include <algorithm>

namespace j2me {
namespace core {

void Interpreter::initExtended() {
    instructionTable[OP_IFNULL] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             JavaValue val = frame->pop();
             if (val.val.ref == nullptr) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_IFNONNULL] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             int16_t offset = (int16_t)codeReader.readU2();
             JavaValue val = frame->pop();
             if (val.val.ref != nullptr) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        } while(0);
        return true;
    };

    instructionTable[OP_MULTIANEWARRAY] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            uint16_t index = codeReader.readU2();
            uint8_t dimensions = codeReader.readU1();
            
            std::vector<int32_t> counts;
            for (int i = 0; i < dimensions; i++) {
                counts.push_back(frame->pop().val.i);
            }
            std::reverse(counts.begin(), counts.end());
            
            // Helper lambda for recursive creation
            std::function<JavaObject*(int)> createArray;
            createArray = [&](int dimIndex) -> JavaObject* {
                if (dimIndex >= dimensions) return nullptr;
                
                int32_t count = counts[dimIndex];
                if (count < 0) throw std::runtime_error("NegativeArraySizeException");
                
                auto arrayObj = HeapManager::getInstance().allocate(nullptr);
                arrayObj->fields.resize(count, 0);
                
                if (dimIndex < dimensions - 1) {
                    for (int i = 0; i < count; i++) {
                         JavaObject* subArray = createArray(dimIndex + 1);
                         arrayObj->fields[i] = (int64_t)subArray;
                    }
                }
                
                return arrayObj;
            };
            
            JavaObject* result = createArray(0);
            
            JavaValue val;
            val.type = JavaValue::REFERENCE;
            val.val.ref = result;
            frame->push(val);
            break;
        } while(0);
        return true;
    };
}

}
}
