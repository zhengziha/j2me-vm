#include "../Interpreter.hpp"
#include "../Opcodes.hpp"
#include "../HeapManager.hpp"
#include "../Logger.hpp"
#include <iostream>
#include <functional>
#include <algorithm>
#include <thread>

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

    instructionTable[OP_MONITORENTER] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            // Pop object reference from operand stack
            JavaValue objVal = frame->pop();
            JavaObject* obj = reinterpret_cast<JavaObject*>(objVal.val.ref);
            
            if (obj == nullptr) {
                throw std::runtime_error("NullPointerException");
            }
            
            // Get or create mutex for this object
            auto it = objectMonitors.find(obj);
            if (it == objectMonitors.end()) {
                objectMonitors[obj] = std::make_unique<std::recursive_mutex>();
                monitorCounts[obj] = 0;
                monitorOwners[obj] = nullptr;
            }
            
            // Check if this thread already owns the monitor
            if (monitorOwners[obj] != thread) {
                // Different thread, need to acquire the lock
                objectMonitors[obj]->lock();
                monitorOwners[obj] = thread;  // Record the owner
            }
            
            // Increment monitor count (for reentrant locking)
            monitorCounts[obj]++;
            
            LOG_DEBUG("[MONITORENTER] Acquired monitor for object: " + std::to_string((long long)obj) + " count: " + std::to_string(monitorCounts[obj]));
            
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_MONITOREXIT] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            // Pop object reference from operand stack
            JavaValue objVal = frame->pop();
            JavaObject* obj = reinterpret_cast<JavaObject*>(objVal.val.ref);
            
            if (obj == nullptr) {
                throw std::runtime_error("NullPointerException");
            }
            
            // Check if monitor is owned by this thread
            if (monitorOwners.find(obj) == monitorOwners.end() || monitorOwners[obj] != thread) {
                // Monitor not held by this thread
                throw std::runtime_error("IllegalMonitorStateException");
            }
            
            // Decrement monitor count
            if (monitorCounts.find(obj) != monitorCounts.end() && monitorCounts[obj] > 0) {
                monitorCounts[obj]--;
                
                // If count reaches 0, release the lock completely
                if (monitorCounts[obj] == 0) {
                    monitorOwners[obj] = nullptr;  // Clear the owner
                    objectMonitors[obj]->unlock();
                    LOG_DEBUG("[MONITOREXIT] Released monitor for object: " + std::to_string((long long)obj));
                } else {
                    LOG_DEBUG("[MONITOREXIT] Monitor count decremented for object: " + std::to_string((long long)obj) + " count: " + std::to_string(monitorCounts[obj]));
                }
            } else {
                // Monitor not held by this thread
                throw std::runtime_error("IllegalMonitorStateException");
            }
            
            break;
        } while(0);
        return true;
    };
}

} // namespace core
} // namespace j2me
