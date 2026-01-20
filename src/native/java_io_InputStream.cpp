#include "java_io_InputStream.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "../loader/JarLoader.hpp"
#include "NativeInputStream.hpp"
#include <iostream>
#include <string>

namespace j2me {
namespace natives {

void registerInputStreamNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();

    // java/io/InputStream.read()I
    registry.registerNative("java/io/InputStream", "read", "()I", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            try {
                    std::cerr << "[Native] InputStream.read()I entered. Stack size: " << frame->size() << std::endl;
                    j2me::core::JavaValue thisVal = frame->pop();
                    
                    j2me::core::JavaValue result;
                    result.type = j2me::core::JavaValue::INT;
                    result.val.i = -1;
                    
                    if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                        j2me::core::JavaObject* inputStreamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                        if (inputStreamObj->fields.size() > 0) {
                            int streamId = (int)inputStreamObj->fields[0];
                            std::cout << "[Native] InputStream.read()I streamId: " << streamId << std::endl;
                            auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                            if (stream) {
                                result.val.i = stream->read();
                                std::cout << "[Native] InputStream.read()I result: " << result.val.i << std::endl;
                            } else {
                                std::cout << "[Native] InputStream.read()I stream not found for id: " << streamId << std::endl;
                            }
                        }
                    }
                    
                    frame->push(result);
                } catch (const std::exception& e) {
                std::cerr << "[Native] InputStream.read()I Exception: " << e.what() << std::endl;
                throw;
            }
        }
    );

    // java/io/InputStream.read([B)I
    registry.registerNative("java/io/InputStream", "read", "([B)I", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue arrayVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = -1;
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* inputStreamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (inputStreamObj->fields.size() > 0) {
                    int streamId = (int)inputStreamObj->fields[0];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    
                    if (stream && arrayVal.type == j2me::core::JavaValue::REFERENCE && arrayVal.val.ref != nullptr) {
                        j2me::core::JavaObject* arrayObj = (j2me::core::JavaObject*)arrayVal.val.ref;
                        if (arrayObj->fields.size() > 0) {
                            int length = (int)arrayObj->fields[0];
                            if (length > 0) {
                                uint8_t* buffer = new uint8_t[length];
                                int bytesRead = stream->read(buffer, length);
                                
                                if (bytesRead > 0) {
                                    for (int i = 0; i < bytesRead && i < length; i++) {
                                        if (arrayObj->fields.size() > (size_t)(i + 1)) {
                                            arrayObj->fields[i + 1] = buffer[i];
                                        }
                                    }
                                    result.val.i = bytesRead;
                                }
                                
                                delete[] buffer;
                            }
                        }
                    }
                }
            }
            
            frame->push(result);
        }
    );

    // java/io/InputStream.close()V
    registry.registerNative("java/io/InputStream", "close", "()V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* inputStreamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (inputStreamObj->fields.size() > 0) {
                    int streamId = (int)inputStreamObj->fields[0];
                    j2me::core::HeapManager::getInstance().removeStream(streamId);
                }
            }
        }
    );
}

} // namespace natives
} // namespace j2me
