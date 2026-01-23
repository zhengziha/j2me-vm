#include "java_lang_StringBuffer.hpp"
#include "java_lang_String.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include <iostream>
#include <map>
#include <string>

namespace j2me {
namespace natives {

// Global map to store StringBuffer content
// Key: StringBuffer ID (stored in the object's first field)
// Value: std::string content
static std::map<int32_t, std::string> stringBufferMap;
static int32_t nextStringBufferId = 1;

void registerStringBufferNatives(j2me::core::NativeRegistry& registry) {
    // registry passed as argument

    // java/lang/StringBuffer.<init>()V
    registry.registerNative("java/lang/StringBuffer", "<init>", "()V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* thisObj = (j2me::core::JavaObject*)thisVal.val.ref;
                
                int32_t id = nextStringBufferId++;
                stringBufferMap[id] = "";
                
                // Ensure we have space for the ID
                if (thisObj->fields.size() < 1) {
                    thisObj->fields.resize(1);
                }
                thisObj->fields[0] = id;
            }
        }
    );

    // java/lang/StringBuffer.append(Ljava/lang/String;)Ljava/lang/StringBuffer;
    registry.registerNative("java/lang/StringBuffer", "append", "(Ljava/lang/String;)Ljava/lang/StringBuffer;", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue strVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* thisObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (thisObj->fields.size() > 0) {
                    int32_t id = (int32_t)thisObj->fields[0];
                    
                    std::string appendStr = "null";
                    if (strVal.type == j2me::core::JavaValue::REFERENCE && strVal.val.ref != nullptr) {
                        appendStr = getJavaString((j2me::core::JavaObject*)strVal.val.ref);
                    } else if (strVal.type == j2me::core::JavaValue::REFERENCE && strVal.val.ref == nullptr) {
                        appendStr = "null";
                    }
                    
                    stringBufferMap[id] += appendStr;
                }
            }
            
            frame->push(thisVal); // Return this
        }
    );

    // java/lang/StringBuffer.append(I)Ljava/lang/StringBuffer;
    registry.registerNative("java/lang/StringBuffer", "append", "(I)Ljava/lang/StringBuffer;", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int32_t val = frame->pop().val.i;
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* thisObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (thisObj->fields.size() > 0) {
                    int32_t id = (int32_t)thisObj->fields[0];
                    stringBufferMap[id] += std::to_string(val);
                }
            }
            
            frame->push(thisVal);
        }
    );

    // java/lang/StringBuffer.append(Ljava/lang/Object;)Ljava/lang/StringBuffer;
    registry.registerNative("java/lang/StringBuffer", "append", "(Ljava/lang/Object;)Ljava/lang/StringBuffer;", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue objVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* thisObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (thisObj->fields.size() > 0) {
                    int32_t id = (int32_t)thisObj->fields[0];
                    
                    std::string str = "null";
                    if (objVal.type == j2me::core::JavaValue::REFERENCE && objVal.val.ref != nullptr) {
                        // Ideally call toString(). For now, simple logic.
                        // Check if it's a String (simple case)
                        j2me::core::JavaObject* obj = (j2me::core::JavaObject*)objVal.val.ref;
                        // We can't easily check type without class name, and getting class name is hard here without loading it.
                        // But we can check if it has "value" field compatible with String? No.
                        // Just use "Object"
                        str = "Object"; 
                        // Note: If we had access to interpreter, we could resolve java/lang/String and check instanceof.
                    }
                    
                    stringBufferMap[id] += str;
                }
            }
            
            frame->push(thisVal);
        }
    );

    // java/lang/StringBuffer.toString()Ljava/lang/String;
    registry.registerNative("java/lang/StringBuffer", "toString", "()Ljava/lang/String;", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* thisObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (thisObj->fields.size() > 0) {
                    int32_t id = (int32_t)thisObj->fields[0];
                    std::string str = stringBufferMap[id];
                    
                    auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                    if (interpreter) {
                        result.val.ref = createJavaString(interpreter, str);
                    }
                }
            }
            frame->push(result);
        }
    );
    // java/lang/StringBuffer.init()V (without brackets, for obfuscated code?)
    registry.registerNative("java/lang/StringBuffer", "init", "()V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* thisObj = (j2me::core::JavaObject*)thisVal.val.ref;
                
                int32_t id = nextStringBufferId++;
                stringBufferMap[id] = "";
                
                if (thisObj->fields.size() < 1) {
                    thisObj->fields.resize(1);
                }
                thisObj->fields[0] = id;
            }
        }
    );
}

}
}
