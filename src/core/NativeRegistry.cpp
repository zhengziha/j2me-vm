#include "NativeRegistry.hpp"
#include "RuntimeTypes.hpp"
#include "HeapManager.hpp"
#include <iostream>

namespace j2me {
namespace core {

NativeRegistry::NativeRegistry() {
    // java/lang/StringBuffer.<init>()V
    registerNative("java/lang/StringBuffer", "<init>", "()V", [](std::shared_ptr<StackFrame> frame) {
        std::cerr << "Native StringBuffer.<init>" << std::endl;
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        if (thisObj) {
            std::string* str = new std::string();
            // Store pointer in first field (ensure size)
            if (thisObj->fields.size() < 1) thisObj->fields.resize(1);
            thisObj->fields[0] = (int64_t)str;
        }
    });

    // java/lang/StringBuffer.append(Ljava/lang/String;)Ljava/lang/StringBuffer;
    registerNative("java/lang/StringBuffer", "append", "(Ljava/lang/String;)Ljava/lang/StringBuffer;", [](std::shared_ptr<StackFrame> frame) {
        std::cerr << "Native StringBuffer.append(String)" << std::endl;
        JavaValue strVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                if (strVal.type == JavaValue::REFERENCE && !strVal.strVal.empty()) {
                     str->append(strVal.strVal);
                } else if (strVal.type == JavaValue::REFERENCE && strVal.val.ref == nullptr) {
                     str->append("null");
                }
            }
        }
        frame->push(thisVal); // Return this
    });

    // java/lang/StringBuffer.append(I)Ljava/lang/StringBuffer;
    registerNative("java/lang/StringBuffer", "append", "(I)Ljava/lang/StringBuffer;", [](std::shared_ptr<StackFrame> frame) {
        std::cerr << "Native StringBuffer.append(int)" << std::endl;
        JavaValue intVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                str->append(std::to_string(intVal.val.i));
            }
        }
        frame->push(thisVal);
    });

    // java/lang/StringBuffer.append(Ljava/lang/Object;)Ljava/lang/StringBuffer;
    registerNative("java/lang/StringBuffer", "append", "(Ljava/lang/Object;)Ljava/lang/StringBuffer;", [](std::shared_ptr<StackFrame> frame) {
        JavaValue objVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                if (objVal.val.ref == nullptr) {
                    str->append("null");
                } else {
                    // Try to use strVal hack if present (e.g. from String)
                    if (objVal.type == JavaValue::REFERENCE && !objVal.strVal.empty()) {
                        str->append(objVal.strVal);
                    } else {
                         // Default object toString
                         JavaObject* obj = (JavaObject*)objVal.val.ref;
                         if (obj) {
                             std::string name = (obj->cls) ? obj->cls->name : "Object";
                             str->append(name + "@" + std::to_string((intptr_t)obj));
                         } else {
                             str->append("null");
                         }
                    }
                }
            }
        }
        frame->push(thisVal);
    });

    // java/lang/StringBuffer.toString()Ljava/lang/String;
    registerNative("java/lang/StringBuffer", "toString", "()Ljava/lang/String;", [](std::shared_ptr<StackFrame> frame) {
        std::cerr << "Native StringBuffer.toString" << std::endl;
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        JavaValue ret;
        ret.type = JavaValue::REFERENCE;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                ret.strVal = *str;
                std::cerr << "StringBuffer result: " << *str << std::endl;
            }
        }
        frame->push(ret);
    });

    // java/lang/String.valueOf(I)Ljava/lang/String;
    registerNative("java/lang/String", "valueOf", "(I)Ljava/lang/String;", [](std::shared_ptr<StackFrame> frame) {
        int val = frame->pop().val.i;
        JavaValue ret;
        ret.type = JavaValue::REFERENCE;
        ret.strVal = std::to_string(val);
        frame->push(ret);
    });
}

void NativeRegistry::registerNative(const std::string& className, const std::string& methodName, const std::string& descriptor, NativeFunction func) {
    registry[makeKey(className, methodName, descriptor)] = func;
}

NativeFunction NativeRegistry::getNative(const std::string& className, const std::string& methodName, const std::string& descriptor) {
    auto it = registry.find(makeKey(className, methodName, descriptor));
    if (it != registry.end()) {
        return it->second;
    }
    return nullptr;
}

std::string NativeRegistry::makeKey(const std::string& className, const std::string& methodName, const std::string& descriptor) {
    return className + "." + methodName + descriptor;
}

} // namespace core
} // namespace j2me
