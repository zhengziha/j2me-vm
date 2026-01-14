#include "java_lang_Class.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "../loader/JarLoader.hpp"
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
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && !nameVal.strVal.empty()) {
                std::string resName = nameVal.strVal;
                // Normalize path (if starts with /, remove it)
                if (resName.size() > 0 && resName[0] == '/') resName = resName.substr(1);
                
                std::cout << "[Class] Loading resource: " << resName << std::endl;
                
                // Get data from JAR
                auto loader = j2me::core::NativeRegistry::getInstance().getJarLoader();
                if (loader && loader->hasFile(resName)) {
                    auto data = loader->getFile(resName);
                    if (data && !data->empty()) {
                        // Create NativeInputStream and store in HeapManager
                        int streamId = j2me::core::HeapManager::getInstance().allocateStream(data->data(), data->size());
                        
                        // Create InputStream object
                        auto inputStreamCls = j2me::core::NativeRegistry::getInstance().getInterpreter()->resolveClass("java/io/InputStream");
                        if (inputStreamCls) {
                            j2me::core::JavaObject* inputStreamObj = j2me::core::HeapManager::getInstance().allocate(inputStreamCls);
                            
                            // Set nativePtr field to stream ID
                            if (inputStreamObj->fields.size() > 0) {
                                inputStreamObj->fields[0] = streamId;
                            }
                            
                            result.val.ref = inputStreamObj;
                            std::cout << "[Class] Resource loaded successfully, stream ID: " << streamId << std::endl;
                        } else {
                            std::cerr << "[Class] Failed to resolve InputStream class" << std::endl;
                        }
                    } else {
                        std::cerr << "[Class] Failed to read resource data" << std::endl;
                    }
                } else {
                    std::cerr << "[Class] Resource not found: " << resName << std::endl;
                }
            }
            
            frame->push(result);
        }
    );
}

} // namespace natives
} // namespace j2me
