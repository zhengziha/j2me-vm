#include "java_io_PrintStream.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include <iostream>

namespace j2me {
namespace natives {

void registerPrintStreamNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();

    // java/io/PrintStream.printlnNative(Ljava/lang/String;)V
    registry.registerNative("java/io/PrintStream", "printlnNative", "(Ljava/lang/String;)V",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue strVal = frame->pop();
            frame->pop(); // this
            
            if (strVal.type == j2me::core::JavaValue::REFERENCE) {
                if (!strVal.strVal.empty()) {
                    std::cout << strVal.strVal << std::endl;
                } else if (strVal.val.ref != nullptr) {
                    auto strObj = static_cast<j2me::core::JavaObject*>(strVal.val.ref);
                    if (strObj && strObj->cls) {
                        auto valueIt = strObj->cls->fieldOffsets.find("value");
                        if (valueIt != strObj->cls->fieldOffsets.end()) {
                            auto valueArray = static_cast<j2me::core::JavaObject*>((void*)strObj->fields[valueIt->second]);
                            if (valueArray) {
                                auto offsetIt = strObj->cls->fieldOffsets.find("offset");
                                auto countIt = strObj->cls->fieldOffsets.find("count");
                                int offset = (offsetIt != strObj->cls->fieldOffsets.end()) ? (int)strObj->fields[offsetIt->second] : 0;
                                int count = (countIt != strObj->cls->fieldOffsets.end()) ? (int)strObj->fields[countIt->second] : valueArray->fields.size();
                                
                                std::string str;
                                for (int i = offset; i < offset + count && i < (int)valueArray->fields.size(); i++) {
                                    str += (char)valueArray->fields[i];
                                }
                                std::cout << str << std::endl;
                            } else {
                                std::cout << "[String object with null value array]" << std::endl;
                            }
                        } else {
                            std::cout << "[String object without value field]" << std::endl;
                        }
                    } else {
                        std::cout << "[String object]" << std::endl;
                    }
                } else {
                    std::cout << "null" << std::endl;
                }
            } else {
                std::cout << std::endl;
            }
        }
    );

    // java/io/PrintStream.printNative(Ljava/lang/String;)V
    registry.registerNative("java/io/PrintStream", "printNative", "(Ljava/lang/String;)V",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue strVal = frame->pop();
            frame->pop(); // this
            
            if (strVal.type == j2me::core::JavaValue::REFERENCE) {
                if (!strVal.strVal.empty()) {
                    std::cout << strVal.strVal;
                } else if (strVal.val.ref != nullptr) {
                    std::cout << "[String object]";
                } else {
                    std::cout << "null";
                }
            }
        }
    );
}

void registerSystemNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();

    // java/lang/System.currentTimeMillisNative()J
    registry.registerNative("java/lang/System", "currentTimeMillisNative", "()J",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::LONG;
            ret.val.l = 0;
            frame->push(ret);
        }
    );
}

}
}
