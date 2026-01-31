#include "java_lang_Class.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "../core/Diagnostics.hpp"
#include "../core/Logger.hpp"
#include "../loader/JarLoader.hpp"
#include "java_lang_String.hpp"
#include <string>

namespace j2me {
namespace natives {

void registerClassNatives(j2me::core::NativeRegistry& registry) {
    // registry passed as argument

    // java/lang/Class.getResourceAsStream(Ljava/lang/String;)Ljava/io/InputStream;
    registry.registerNative("java/lang/Class", "getResourceAsStream", "(Ljava/lang/String;)Ljava/io/InputStream;", 
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            LOG_DEBUG("[Class] getResourceAsStream called");
            j2me::core::JavaValue nameVal = frame->pop();
            frame->pop(); // this (Class object)
            LOG_DEBUG("[Class] Args popped. Name type: " + std::to_string(nameVal.type));
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && nameVal.val.ref != nullptr) {
                auto strObj = (j2me::core::JavaObject*)nameVal.val.ref;
                std::string resName = getJavaString(strObj);
                
                LOG_DEBUG("[Class] Requesting resource: " + resName);
                
                // Remove leading slash if present
                if (resName.size() > 0 && resName[0] == '/') resName = resName.substr(1);
                
                LOG_DEBUG("[Class] Loading resource: " + resName);
                
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
                        } else {
                            LOG_ERROR("[Class] WARNING: InputStream object has 0 fields! Cannot store streamId.");
                        }
                        
                        result.val.ref = streamObj;
                        LOG_DEBUG("[Class] Resource loaded successfully, stream ID: " + std::to_string(streamId) + " Size: " + std::to_string(data->size()));
                        
                        // Debug: Print first 16 bytes
                        std::string debugHeader = "DEBUG_HEADER_PRINT: ";
                        for (size_t i = 0; i < std::min((size_t)16, data->size()); i++) {
                            char buf[16];
                            snprintf(buf, sizeof(buf), "%02X ", (*data)[i]);
                            debugHeader += buf;
                        }
                        LOG_DEBUG(debugHeader);
                    } else {
                        LOG_DEBUG("[Class] Resource not found (data null): " + resName);
                        j2me::core::Diagnostics::getInstance().onResourceNotFound(resName);
                    }
                } else {
                    LOG_DEBUG("[Class] Resource not found in JAR: " + resName);
                    j2me::core::Diagnostics::getInstance().onResourceNotFound(resName);
                }
            }
            
            frame->push(result);
        }
    );
}

} // namespace natives
} // namespace j2me
