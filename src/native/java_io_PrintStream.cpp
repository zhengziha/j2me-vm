#include "java_io_PrintStream.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "java_lang_String.hpp"
#include <iostream>

namespace j2me {
namespace natives {

void registerPrintStreamNatives(j2me::core::NativeRegistry& registry) {
    // registry passed as argument

    // java/io/PrintStream.println(Ljava/lang/String;)V
    registry.registerNative("java/io/PrintStream", "printlnNative", "(Ljava/lang/String;)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue strVal = frame->pop();
            frame->pop(); // this
            
            if (strVal.type == j2me::core::JavaValue::REFERENCE) {
                if (strVal.val.ref != nullptr) {
                    auto strObj = static_cast<j2me::core::JavaObject*>(strVal.val.ref);
                    if (strObj && strObj->cls) {
                        std::string str = getJavaString(strObj);
                        std::cout << str << std::endl;
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
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue strVal = frame->pop();
            frame->pop(); // this
            
            if (strVal.type == j2me::core::JavaValue::REFERENCE) {
                if (!strVal.strVal.empty()) {
                    std::cout << strVal.strVal;
                } else if (strVal.val.ref != nullptr) {
                    auto strObj = static_cast<j2me::core::JavaObject*>(strVal.val.ref);
                    if (strObj && strObj->cls) {
                        std::string str = getJavaString(strObj);
                        std::cout << str;
                    } else {
                        std::cout << "[String object]";
                    }
                } else {
                    std::cout << "null";
                }
            }
        }
    );
}

}
}
