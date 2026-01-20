#include "java_lang_Object.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "java_lang_String.hpp"
#include <iostream>

namespace j2me {
namespace natives {

void registerObjectNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();

    // java/lang/Object.getClass()Ljava/lang/Class;
    registry.registerNative("java/lang/Object", "getClass", "()Ljava/lang/Class;", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue objVal = frame->pop(); // this
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::REFERENCE;
            ret.val.ref = nullptr;
            
            if (objVal.val.ref == nullptr) {
                std::cerr << "NullPointerException in Object.getClass()" << std::endl;
            } else {
                j2me::core::JavaObject* obj = static_cast<j2me::core::JavaObject*>(objVal.val.ref);
                
                auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                if (interpreter) {
                    auto classCls = interpreter->resolveClass("java/lang/Class");
                    if (classCls) {
                        auto classObj = j2me::core::HeapManager::getInstance().allocate(classCls);
                        
                        // Set 'name' field
                        std::string className = obj->cls->name;
                        // Convert internal name (slashes) to binary name (dots)
                        for (auto& c : className) {
                            if (c == '/') c = '.';
                        }
                        
                        auto stringObj = createJavaString(interpreter, className);
                        
                        auto nameIt = classCls->fieldOffsets.find("name");
                        if (nameIt != classCls->fieldOffsets.end()) {
                            classObj->fields[nameIt->second] = (int64_t)stringObj;
                        }
                        
                        ret.val.ref = classObj;
                        std::cout << "[Object] getClass returning Class object for " << className << std::endl;
                    } else {
                        std::cerr << "[Object] Failed to resolve java/lang/Class" << std::endl;
                    }
                }
            }
            frame->push(ret);
        }
    );

    // java/lang/Object.hashCode()I
    registry.registerNative("java/lang/Object", "hashCode", "()I", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue objVal = frame->pop(); // this
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            // Use pointer address as hash code
            ret.val.i = (int32_t)(intptr_t)objVal.val.ref;
            frame->push(ret);
        }
    );

    // java/lang/Object.toString()Ljava/lang/String;
    registry.registerNative("java/lang/Object", "toString", "()Ljava/lang/String;", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue objVal = frame->pop(); // this
            
            std::string str = "Object@" + std::to_string((intptr_t)objVal.val.ref);
            if (objVal.val.ref) {
                j2me::core::JavaObject* obj = static_cast<j2me::core::JavaObject*>(objVal.val.ref);
                if (obj->cls) {
                    str = obj->cls->name + "@" + std::to_string((intptr_t)objVal.val.ref);
                }
            }
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::REFERENCE;
            ret.strVal = str;
            frame->push(ret);
        }
    );
}

} // namespace natives
} // namespace j2me
