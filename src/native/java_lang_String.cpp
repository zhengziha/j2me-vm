#include "java_lang_String.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include <iostream>
#include <string>

namespace j2me {
namespace natives {

void registerStringNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();

    // java/lang/String.getBytes()[B
    registry.registerNative("java/lang/String", "getBytes", "()[B",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            
            auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
            auto arrayCls = interpreter->resolveClass("[B");
            if (arrayCls) {
                std::string strValue;
                
                if (thisVal.type == j2me::core::JavaValue::REFERENCE) {
                    strValue = thisVal.strVal;
                    std::cout << "[String.getBytes] String value: '" << strValue << "' (length: " << strValue.length() << ")" << std::endl;
                }
                
                auto arrayObj = j2me::core::HeapManager::getInstance().allocate(arrayCls);
                arrayObj->fields.resize(strValue.length());
                for (size_t i = 0; i < strValue.length(); i++) {
                    arrayObj->fields[i] = static_cast<uint8_t>(strValue[i]);
                }
                result.val.ref = arrayObj;
                std::cout << "[String.getBytes] Created byte array with " << arrayObj->fields.size() << " bytes" << std::endl;
            }
            
            frame->push(result);
        }
    );

    // java/lang/String.valueOf(I)Ljava/lang/String;
    registry.registerNative("java/lang/String", "valueOf", "(I)Ljava/lang/String;",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue intVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            
            int value = intVal.val.i;
            auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
            auto stringCls = interpreter->resolveClass("java/lang/String");
            if (stringCls) {
                auto stringObj = j2me::core::HeapManager::getInstance().allocate(stringCls);
                result.val.ref = stringObj;
                result.strVal = std::to_string(value);
            }
            
            frame->push(result);
        }
    );

    // java/lang/String.valueOf(J)Ljava/lang/String;
    registry.registerNative("java/lang/String", "valueOf", "(J)Ljava/lang/String;",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue longVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            
            long value = longVal.val.l;
            auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
            auto stringCls = interpreter->resolveClass("java/lang/String");
            if (stringCls) {
                auto stringObj = j2me::core::HeapManager::getInstance().allocate(stringCls);
                result.val.ref = stringObj;
                result.strVal = std::to_string(value);
            }
            
            frame->push(result);
        }
    );

    // java/lang/String.<init>([B)V
    registry.registerNative("java/lang/String", "<init>", "([B)V",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue byteArrayVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                auto thisObj = static_cast<j2me::core::JavaObject*>(thisVal.val.ref);
                if (thisObj && thisObj->cls) {
                    if (byteArrayVal.type == j2me::core::JavaValue::REFERENCE && byteArrayVal.val.ref != nullptr) {
                        auto byteArrayObj = static_cast<j2me::core::JavaObject*>(byteArrayVal.val.ref);
                        
                        auto valueIt = thisObj->cls->fieldOffsets.find("value");
                        if (valueIt != thisObj->cls->fieldOffsets.end()) {
                            thisObj->fields[valueIt->second] = reinterpret_cast<intptr_t>(byteArrayObj);
                            
                            auto offsetIt = thisObj->cls->fieldOffsets.find("offset");
                            if (offsetIt != thisObj->cls->fieldOffsets.end()) {
                                thisObj->fields[offsetIt->second] = 0;
                            }
                            
                            auto countIt = thisObj->cls->fieldOffsets.find("count");
                            if (countIt != thisObj->cls->fieldOffsets.end()) {
                                thisObj->fields[countIt->second] = byteArrayObj->fields.size();
                            }
                            
                            std::cout << "[String.<init>] Created String from byte array with " << byteArrayObj->fields.size() << " bytes" << std::endl;
                        }
                    }
                }
            }
        }
    );
}

}
}
