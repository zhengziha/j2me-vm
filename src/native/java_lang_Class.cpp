#include "java_lang_Class.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include <iostream>
#include <string>

namespace j2me {
namespace natives {

void registerClassNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();

    // java/lang/Class.getResourceAsStream(Ljava/lang/String;)Ljava/io/InputStream;
    registry.registerNative("java/lang/Class", "getResourceAsStream", "(Ljava/lang/String;)Ljava/io/InputStream;", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop();
            frame->pop(); // this (Class object)
            
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && !nameVal.strVal.empty()) {
                std::string resName = nameVal.strVal;
                // Normalize path (if starts with /, remove it)
                if (resName.size() > 0 && resName[0] == '/') resName = resName.substr(1);
                
                std::cout << "[Class] Loading resource: " << resName << std::endl;
                
                // For Phase 4, we just return null or a dummy InputStream
                // To support it fully, we need to:
                // 1. Create a java/io/InputStream object (or subclass like ByteArrayInputStream)
                // 2. Read data from JAR using JarLoader
                // 3. Store data in the stream object
                
                // Returning null for now as placeholder
                j2me::core::JavaValue nullVal;
                nullVal.type = j2me::core::JavaValue::REFERENCE;
                nullVal.val.ref = nullptr;
                frame->push(nullVal);
            } else {
                 j2me::core::JavaValue nullVal;
                 nullVal.type = j2me::core::JavaValue::REFERENCE;
                 nullVal.val.ref = nullptr;
                 frame->push(nullVal);
            }
        }
    );
}

} // namespace natives
} // namespace j2me
