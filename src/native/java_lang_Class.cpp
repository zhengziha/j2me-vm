#include "java_lang_Class.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "../core/Diagnostics.hpp"
#include "../loader/JarLoader.hpp"
#include "java_lang_String.hpp"
#include <iostream>
#include <string>

namespace j2me {
namespace natives {

void registerClassNatives(j2me::core::NativeRegistry& registry) {
    // registry passed as argument

    // java/lang/Class.getResourceAsStream(Ljava/lang/String;)Ljava/io/InputStream;
    registry.registerNative("java/lang/Class", "getResourceAsStream", "(Ljava/lang/String;)Ljava/io/InputStream;", 
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            std::cout << "[Class] getResourceAsStream called" << std::endl;
            j2me::core::JavaValue nameVal = frame->pop();
            frame->pop(); // this (Class object)
            std::cout << "[Class] Args popped. Name type: " << nameVal.type << std::endl;
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && nameVal.val.ref != nullptr) {
                auto strObj = (j2me::core::JavaObject*)nameVal.val.ref;
                std::string resName = getJavaString(strObj);
                
                std::cout << "[Class] Requesting resource: " << resName << std::endl;
                
                // Remove leading slash if present
                if (resName.size() > 0 && resName[0] == '/') resName = resName.substr(1);
                
                std::cout << "[Class] Loading resource: " << resName << std::endl;
                
                auto loader = j2me::core::NativeRegistry::getInstance().getJarLoader();
                if (loader && loader->hasFile(resName)) {
                    auto data = loader->getFile(resName);
                    if (data) {
                        // Create NativeInputStream via HeapManager
                        int streamId = j2me::core::HeapManager::getInstance().allocateStream(data->data(), data->size());
                        
                        auto inputStreamCls = registry.getInterpreter()->resolveClass("java/io/InputStream");
                        auto streamObj = j2me::core::HeapManager::getInstance().allocate(inputStreamCls);
                        if (streamObj->fields.size() > 0) {
                            streamObj->fields[0] = streamId;
                        }
                        
                        result.val.ref = streamObj;
                        std::cout << "[Class] Resource loaded successfully, stream ID: " << streamId << " Size: " << data->size() << std::endl;
                        
                        // Debug: Print first 16 bytes
                        std::cout << "DEBUG_HEADER_PRINT: ";
                        for (size_t i = 0; i < std::min((size_t)16, data->size()); i++) {
                            char buf[16];
                            snprintf(buf, sizeof(buf), "%02X ", (*data)[i]);
                            std::cout << buf;
                        }
                        std::cout << std::endl;
                    } else {
                        std::cout << "[Class] Resource not found (data null): " << resName << std::endl;
                        j2me::core::Diagnostics::getInstance().onResourceNotFound(resName);
                    }
                } else {
                    std::cout << "[Class] Resource not found in JAR: " << resName << std::endl;
                    j2me::core::Diagnostics::getInstance().onResourceNotFound(resName);
                }
            }
            
            frame->push(result);
        }
    );
}

} // namespace natives
} // namespace j2me
