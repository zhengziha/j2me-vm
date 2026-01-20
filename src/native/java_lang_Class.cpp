#include "java_lang_Class.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "../loader/JarLoader.hpp"
#include "java_lang_String.hpp"
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
            j2me::core::JavaValue thisVal = frame->pop(); // this (Class object)
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            
            std::string resName;
            if (nameVal.type == j2me::core::JavaValue::REFERENCE) {
                if (!nameVal.strVal.empty()) {
                    resName = nameVal.strVal;
                } else if (nameVal.val.ref != nullptr) {
                    resName = getJavaString((j2me::core::JavaObject*)nameVal.val.ref);
                }
            }
            
            if (!resName.empty()) {
                // Normalize path (if starts with /, remove it)
                if (resName.size() > 0 && resName[0] == '/') {
                    resName = resName.substr(1);
                } else {
                    // Relative path: prepend package of this class
                    // We need to get the name of the class this Class object represents
                    if (thisVal.val.ref != nullptr) {
                        j2me::core::JavaObject* classObj = (j2me::core::JavaObject*)thisVal.val.ref;
                        auto nameIt = classObj->cls->fieldOffsets.find("name");
                        if (nameIt != classObj->cls->fieldOffsets.end()) {
                            int64_t nameRef = classObj->fields[nameIt->second];
                            if (nameRef != 0) {
                                std::string className = getJavaString((j2me::core::JavaObject*)nameRef);
                                // className is like "java.lang.String"
                                // Convert to path "java/lang/String"
                                for (auto& c : className) {
                                    if (c == '.') c = '/';
                                }
                                // Remove class name, keep package
                                auto lastSlash = className.find_last_of('/');
                                if (lastSlash != std::string::npos) {
                                    resName = className.substr(0, lastSlash + 1) + resName;
                                } else {
                                    // Default package, no prefix needed (unless strictly required?)
                                }
                            }
                        }
                    }
                }
                
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
