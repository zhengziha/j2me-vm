#include "java_io_InputStream.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "../loader/JarLoader.hpp"
#include "NativeInputStream.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace j2me {
namespace natives {

void registerInputStreamNatives(j2me::core::NativeRegistry& registry) {
    // registry passed as argument

    // java/io/InputStream.read()I
    registry.registerNative("java/io/InputStream", "read", "()I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            try {
                j2me::core::JavaValue thisVal = frame->pop();
                
                j2me::core::JavaValue result;
                result.type = j2me::core::JavaValue::INT;
                result.val.i = -1;
                
                if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                    j2me::core::JavaObject* inputStreamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                    if (inputStreamObj->fields.size() > 0) {
                        int streamId = (int)inputStreamObj->fields[0];
                        auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                        
                        if (stream) {
                            result.val.i = stream->read();
                        } else {
                            std::cout << "[InputStream.read()I] ERROR - stream not found for id: " << streamId << std::endl;
                        }
                    } else {
                        std::cout << "[InputStream.read()I] ERROR - inputStreamObj has no fields" << std::endl;
                    }
                } else {
                    std::cout << "[InputStream.read()I] ERROR - Invalid this object" << std::endl;
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
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
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
                    
                    j2me::core::JavaObject* arrayObj = nullptr;
                    size_t arrayLength = 0;
                    
                    if (arrayVal.type == j2me::core::JavaValue::REFERENCE && arrayVal.val.ref != nullptr) {
                        arrayObj = (j2me::core::JavaObject*)arrayVal.val.ref;
                        arrayLength = arrayObj->fields.size();
                    }
                    
                    if (stream && arrayObj != nullptr) {
                        
                        if (arrayLength > 0) {
                            std::vector<uint8_t> buffer(arrayLength);
                            int bytesRead = stream->read(buffer.data(), (int)arrayLength);
                            
                            if (bytesRead > 0) {
                                for (size_t i = 0; i < (size_t)bytesRead; i++) {
                                    arrayObj->fields[i] = (int64_t)buffer[i];
                                }
                                result.val.i = bytesRead;
                            }
                            

                        } else {
                            result.val.i = 0;
                            std::cout << "[InputStream.read([B)I] AFTER - streamId: " << streamId << " arrayLength: 0, returning 0" << std::endl;
                        }
                    } else {
                        std::cout << "[InputStream.read([B)I] ERROR - stream or array object not found - stream: " << (stream ? "valid" : "null") << " arrayObj: " << (arrayObj ? "valid" : "null") << std::endl;
                    }
                } else {
                    std::cout << "[InputStream.read([B)I] ERROR - inputStreamObj has no fields" << std::endl;
                }
            } else {
                std::cout << "[InputStream.read([B)I] ERROR - Invalid this or array object" << std::endl;
            }
            
            frame->push(result);
        }
    );

    // java/io/InputStream.read([BII)I
    registry.registerNative("java/io/InputStream", "read", "([BII)I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int len = frame->pop().val.i;
            int off = frame->pop().val.i;
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
                    
                    j2me::core::JavaObject* arrayObj = nullptr;
                    size_t arrayLen = 0;
                    
                    if (arrayVal.type == j2me::core::JavaValue::REFERENCE && arrayVal.val.ref != nullptr) {
                        arrayObj = (j2me::core::JavaObject*)arrayVal.val.ref;
                        arrayLen = arrayObj->fields.size();
                    }

                    std::cout << "[InputStream.read([BII)I] BEFORE - streamId: " << streamId << " path: " << stream->getFilePath() << " method: read([BII)I" << " offset: " << off << " len: " << len << " arrayLen: " << arrayLen << std::endl;
                    
                    if (stream && arrayObj != nullptr) {
                        if (off < 0 || len < 0 || off + len > (int)arrayLen) {
                            std::cout << "[InputStream.read([BII)I] ERROR - Invalid parameters: off: " << off << " len: " << len << " arrayLen: " << arrayLen << std::endl;
                             // IndexOutOfBoundsException - effectively return -1 for now
                        } else if (len == 0) {
                            result.val.i = 0;
                            std::cout << "[InputStream.read([BII)I] AFTER - streamId: " << streamId << " path: " << stream->getFilePath() << " len==0, returning 0" << std::endl;
                        } else {
                            std::cout << "[InputStream.read([BII)I] Calling stream->read(buffer, " << len << ") with stream object: " << stream << " path: " << stream->getFilePath() << std::endl;
                            std::vector<uint8_t> buffer(len);
                            int bytesRead = stream->read(buffer.data(), len);
                            
                            if (bytesRead > 0) {
                                for (int i = 0; i < bytesRead; i++) {
                                    arrayObj->fields[off + i] = (int64_t)buffer[i];
                                }
                                result.val.i = bytesRead;
                            }
                            

                        }
                    } else {
                        std::cout << "[InputStream.read([BII)I] ERROR - stream or array object not found - stream: " << (stream ? "valid" : "null") << " arrayObj: " << (arrayObj ? "valid" : "null") << std::endl;
                    }
                } else {
                    std::cout << "[InputStream.read([BII)I] ERROR - inputStreamObj has no fields" << std::endl;
                }
            } else {
                std::cout << "[InputStream.read([BII)I] ERROR - Invalid this or array object" << std::endl;
            }
            frame->push(result);
        }
    );

    // java/io/InputStream.close()V
    registry.registerNative("java/io/InputStream", "close", "()V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* inputStreamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (inputStreamObj->fields.size() > 0) {
                    int streamId = (int)inputStreamObj->fields[0];
                    std::cout << "[InputStream.close()V] BEFORE - streamId: " << streamId << " method: close()V" << std::endl;
                    j2me::core::HeapManager::getInstance().removeStream(streamId);
                    std::cout << "[InputStream.close()V] AFTER - streamId: " << streamId << " stream removed" << std::endl;
                } else {
                    std::cout << "[InputStream.close()V] ERROR - inputStreamObj has no fields" << std::endl;
                }
            } else {
                std::cout << "[InputStream.close()V] ERROR - Invalid this object" << std::endl;
            }
        }
    );

    // java/io/InputStream.available()I
    registry.registerNative("java/io/InputStream", "available", "()I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = 0;
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* inputStreamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (inputStreamObj->fields.size() > 0) {
                    int streamId = (int)inputStreamObj->fields[0];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    
                    if (stream) {
                        result.val.i = stream->available();
                    } else {
                        std::cout << "[InputStream.available()I] ERROR - stream not found for id: " << streamId << std::endl;
                    }
                } else {
                    std::cout << "[InputStream.available()I] ERROR - inputStreamObj has no fields" << std::endl;
                }
            } else {
                std::cout << "[InputStream.available()I] ERROR - Invalid this object" << std::endl;
            }
            frame->push(result);
        }
    );

    // java/io/InputStream.skip(J)J
    registry.registerNative("java/io/InputStream", "skip", "(J)J", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int64_t n = frame->pop().val.l;
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::LONG;
            result.val.l = 0;
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* inputStreamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (inputStreamObj->fields.size() > 0) {
                    int streamId = (int)inputStreamObj->fields[0];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    
                    if (stream) {
                        result.val.l = stream->skip(n);
                    } else {
                        std::cout << "[InputStream.skip(J)J] ERROR - stream not found for id: " << streamId << std::endl;
                    }
                } else {
                    std::cout << "[InputStream.skip(J)J] ERROR - inputStreamObj has no fields" << std::endl;
                }
            } else {
                std::cout << "[InputStream.skip(J)J] ERROR - Invalid this object" << std::endl;
            }
            frame->push(result);
        }
    );

    // java/io/InputStream.markSupported()Z
    registry.registerNative("java/io/InputStream", "markSupported", "()Z", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop(); // this
            
            bool supported = false;
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* inputStreamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (inputStreamObj->fields.size() > 0) {
                     // Check if it's our NativeInputStream
                     // Ideally we should check class type, but for now we assume if it has handle it is ours.
                     int streamId = (int)inputStreamObj->fields[0];
                     auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                     
                     if (stream) {
                         supported = stream->markSupported();
                     } else {
                         std::cout << "[InputStream.markSupported()Z] ERROR - stream not found for id: " << streamId << std::endl;
                     }
                } else {
                    std::cout << "[InputStream.markSupported()Z] ERROR - inputStreamObj has no fields" << std::endl;
                }
            } else {
                std::cout << "[InputStream.markSupported()Z] ERROR - Invalid this object" << std::endl;
            }

            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = supported ? 1 : 0;
            frame->push(result);
        }
    );

    // java/io/InputStream.mark(I)V
    registry.registerNative("java/io/InputStream", "mark", "(I)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int readlimit = frame->pop().val.i;
            j2me::core::JavaValue thisVal = frame->pop(); // this
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* inputStreamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (inputStreamObj->fields.size() > 0) {
                    int streamId = (int)inputStreamObj->fields[0];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    
                    if (stream) {
                        stream->mark(readlimit);
                    } else {
                        std::cout << "[InputStream.mark(I)V] ERROR - stream not found for id: " << streamId << std::endl;
                    }
                } else {
                    std::cout << "[InputStream.mark(I)V] ERROR - inputStreamObj has no fields" << std::endl;
                }
            } else {
                std::cout << "[InputStream.mark(I)V] ERROR - Invalid this object" << std::endl;
            }
        }
    );

    // java/io/InputStream.reset()V
    registry.registerNative("java/io/InputStream", "reset", "()V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop(); // this
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* inputStreamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (inputStreamObj->fields.size() > 0) {
                    int streamId = (int)inputStreamObj->fields[0];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    
                    if (stream) {
                        stream->reset();
                    } else {
                        std::cout << "[InputStream.reset()V] ERROR - stream not found for id: " << streamId << std::endl;
                    }
                } else {
                    std::cout << "[InputStream.reset()V] ERROR - inputStreamObj has no fields" << std::endl;
                }
            } else {
                std::cout << "[InputStream.reset()V] ERROR - Invalid this object" << std::endl;
            }
        }
    );
}

} // namespace natives
} // namespace j2me
