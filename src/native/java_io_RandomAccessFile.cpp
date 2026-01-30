#include "java_io_RandomAccessFile.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "../core/Diagnostics.hpp"
#include "../loader/JarLoader.hpp"
#include "java_lang_String.hpp"
#include "NativeInputStream.hpp"
#include <iostream>
#include <string>

namespace j2me {
namespace natives {

void registerRandomAccessFileNatives(j2me::core::NativeRegistry& registry) {
    
    registry.registerNative("java/io/RandomAccessFile", "openNative", "(Ljava/lang/String;Ljava/lang/String;)V", 
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            std::cout << "[RandomAccessFile] openNative called" << std::endl;
            
            j2me::core::JavaValue modeVal = frame->pop();
            j2me::core::JavaValue nameVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && nameVal.val.ref != nullptr) {
                auto nameStr = (j2me::core::JavaObject*)nameVal.val.ref;
                std::string name = getJavaString(nameStr);
                
                std::cout << "[RandomAccessFile] Opening resource: " << name << std::endl;
                
                auto loader = j2me::core::NativeRegistry::getInstance().getJarLoader();
                if (loader && loader->hasFile(name)) {
                    auto data = loader->getFile(name);
                    if (data) {
                        int streamId = j2me::core::HeapManager::getInstance().allocateStreamWithPath(data->data(), data->size(), name);
                        
                        if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                            auto thisObj = (j2me::core::JavaObject*)thisVal.val.ref;
                            if (thisObj->fields.size() > 0) {
                                thisObj->fields[0] = streamId;
                            }
                        }
                        
                        std::cout << "[RandomAccessFile] Opened successfully, stream ID: " << streamId << " Size: " << data->size() << std::endl;
                    } else {
                        std::cout << "[RandomAccessFile] Resource not found (data null): " << name << std::endl;
                        j2me::core::Diagnostics::getInstance().onResourceNotFound(name);
                    }
                } else {
                    std::cout << "[RandomAccessFile] Resource not found in JAR: " << name << std::endl;
                    j2me::core::Diagnostics::getInstance().onResourceNotFound(name);
                }
            }
        }
    );
    
    registry.registerNative("java/io/RandomAccessFile", "closeNative", "()V", 
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            std::cout << "[RandomAccessFile] closeNative called" << std::endl;
            
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                auto thisObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (thisObj->fields.size() > 0) {
                    int streamId = thisObj->fields[0];
                    j2me::core::HeapManager::getInstance().removeStream(streamId);
                    thisObj->fields[0] = 0;
                }
            }
        }
    );
    
    registry.registerNative("java/io/RandomAccessFile", "readNative", "()I", 
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = -1;
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                auto thisObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (thisObj->fields.size() > 0) {
                    int streamId = thisObj->fields[0];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    if (stream) {
                        result.val.i = stream->read();
                    }
                }
            }
            
            frame->push(result);
        }
    );
    
    registry.registerNative("java/io/RandomAccessFile", "readNative", "([BII)I", 
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue lenVal = frame->pop();
            j2me::core::JavaValue offVal = frame->pop();
            j2me::core::JavaValue bufVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = -1;
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr &&
                bufVal.type == j2me::core::JavaValue::REFERENCE && bufVal.val.ref != nullptr) {
                auto thisObj = (j2me::core::JavaObject*)thisVal.val.ref;
                auto bufObj = (j2me::core::JavaObject*)bufVal.val.ref;
                
                if (thisObj->fields.size() > 0) {
                    int streamId = thisObj->fields[0];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    if (stream && bufObj->fields.size() > 0) {
                        size_t arrayLen = bufObj->fields.size();
                        int off = offVal.val.i;
                        int len = lenVal.val.i;
                        
                        if (off >= 0 && len >= 0 && off + len <= (int)arrayLen) {
                            if (len == 0) {
                                result.val.i = 0;
                            } else {
                                std::vector<uint8_t> buffer(len);
                                int bytesRead = stream->read(buffer.data(), len);
                                
                                if (bytesRead > 0) {
                                    for (int i = 0; i < bytesRead; i++) {
                                        bufObj->fields[off + i] = (int64_t)buffer[i];
                                    }
                                    result.val.i = bytesRead;
                                }
                            }
                        }
                    }
                }
            }
            
            frame->push(result);
        }
    );
    
    registry.registerNative("java/io/RandomAccessFile", "writeNative", "(I)V", 
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue bVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            std::cout << "[RandomAccessFile] writeNative called (write not supported for JAR resources)" << std::endl;
        }
    );
    
    registry.registerNative("java/io/RandomAccessFile", "writeNative", "([BII)V", 
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue lenVal = frame->pop();
            j2me::core::JavaValue offVal = frame->pop();
            j2me::core::JavaValue bufVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            std::cout << "[RandomAccessFile] writeNative called (write not supported for JAR resources)" << std::endl;
        }
    );
    
    registry.registerNative("java/io/RandomAccessFile", "seekNative", "(J)V", 
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue posVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            std::cout << "[RandomAccessFile] seekNative called, position: " << posVal.val.l << std::endl;
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                auto thisObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (thisObj->fields.size() > 0) {
                    int streamId = thisObj->fields[0];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    if (stream) {
                        stream->seek(posVal.val.l);
                    }
                }
            }
        }
    );
    
    registry.registerNative("java/io/RandomAccessFile", "getFilePointerNative", "()J", 
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::LONG;
            result.val.l = 0;
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                auto thisObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (thisObj->fields.size() > 0) {
                    int streamId = thisObj->fields[0];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    if (stream) {
                        result.val.l = stream->getPosition();
                    }
                }
            }
            
            frame->push(result);
        }
    );
    
    registry.registerNative("java/io/RandomAccessFile", "lengthNative", "()J", 
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::LONG;
            result.val.l = 0;
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                auto thisObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (thisObj->fields.size() > 0) {
                    int streamId = thisObj->fields[0];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    if (stream) {
                        result.val.l = stream->getSize();
                    }
                }
            }
            
            frame->push(result);
        }
    );
}

} 
}

extern "C" void initRandomAccessFile(j2me::core::NativeRegistry& registry) {
    j2me::natives::registerRandomAccessFileNatives(registry);
}
