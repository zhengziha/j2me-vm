#include "java_lang_String.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include <iostream>
#include <string>

namespace j2me {
namespace natives {

j2me::core::JavaObject* createJavaString(j2me::core::Interpreter* interpreter, const std::string& str) {
    if (!interpreter) return nullptr;
    
    auto stringCls = interpreter->resolveClass("java/lang/String");
    if (!stringCls) return nullptr;
    
    auto stringObj = j2me::core::HeapManager::getInstance().allocate(stringCls);
    
    // Create char array for value
    // We try [C first (standard), then [B (fallback)
    auto arrayCls = interpreter->resolveClass("[C");
    if (!arrayCls) arrayCls = interpreter->resolveClass("[B");
    
    if (arrayCls) {
        auto arrayObj = j2me::core::HeapManager::getInstance().allocate(arrayCls);
        arrayObj->fields.resize(str.length());
        for (size_t i = 0; i < str.length(); i++) {
            arrayObj->fields[i] = (uint16_t)(uint8_t)str[i]; 
        }
        
        // Set value field
        auto valueIt = stringCls->fieldOffsets.find("value");
        if (valueIt != stringCls->fieldOffsets.end()) {
            stringObj->fields[valueIt->second] = (int64_t)arrayObj;
        }
        
        // Set count field (if exists)
        auto countIt = stringCls->fieldOffsets.find("count");
        if (countIt != stringCls->fieldOffsets.end()) {
             stringObj->fields[countIt->second] = str.length();
        }
        
        // Set offset field (if exists)
        auto offsetIt = stringCls->fieldOffsets.find("offset");
        if (offsetIt != stringCls->fieldOffsets.end()) {
             stringObj->fields[offsetIt->second] = 0;
        }
    }
    
    return stringObj;
}

std::string getJavaString(j2me::core::JavaObject* strObj) {
    if (!strObj || !strObj->cls) return "";
    
    // Try to get from value field
    auto valueIt = strObj->cls->fieldOffsets.find("value");
    if (valueIt != strObj->cls->fieldOffsets.end()) {
        int64_t arrayRef = strObj->fields[valueIt->second];
        if (arrayRef != 0) {
            auto arrayObj = (j2me::core::JavaObject*)arrayRef;
            if (!arrayObj) return "";
            
            // Handle offset and count if they exist
            size_t offset = 0;
            size_t count = arrayObj->fields.size();
            
            auto offsetIt = strObj->cls->fieldOffsets.find("offset");
            if (offsetIt != strObj->cls->fieldOffsets.end()) {
                offset = (size_t)strObj->fields[offsetIt->second];
            }
            
            auto countIt = strObj->cls->fieldOffsets.find("count");
            if (countIt != strObj->cls->fieldOffsets.end()) {
                count = (size_t)strObj->fields[countIt->second];
            }
            
            std::string res;
            if (offset < arrayObj->fields.size()) {
                size_t actualCount = std::min(count, arrayObj->fields.size() - offset);
                res.reserve(actualCount);
                for (size_t i = 0; i < actualCount; i++) {
                    res += (char)arrayObj->fields[offset + i];
                }
            }
            return res;
        }
    }
    
    return "";
}

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
