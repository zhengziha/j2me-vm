#include "java_io_ResourceInputStream.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "../core/Diagnostics.hpp"
#include "../core/Logger.hpp"
#include "NativeInputStream.hpp"
#include <string>

namespace j2me {
namespace natives {

void registerResourceInputStreamNatives(j2me::core::NativeRegistry& registry) {
    // java/io/ResourceInputStream.nativeRead()I
    registry.registerNative("java/io/ResourceInputStream", "nativeRead", "()I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = -1;
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* streamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (streamObj->fields.size() > 1) {
                    int streamId = (int)streamObj->fields[1];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    
                    if (stream) {
                        result.val.i = stream->read();
                    } else {
                        LOG_ERROR("[ResourceInputStream.nativeRead()I] ERROR - stream not found for id: " + std::to_string(streamId));
                    }
                } else {
                    LOG_ERROR("[ResourceInputStream.nativeRead()I] ERROR - streamObj has no fields");
                }
            } else {
                LOG_ERROR("[ResourceInputStream.nativeRead()I] ERROR - Invalid this object");
            }
            
            frame->push(result);
        }
    );

    // java/io/ResourceInputStream.nativeRead([BII)I
    registry.registerNative("java/io/ResourceInputStream", "nativeRead", "([BII)I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int len = frame->pop().val.i;
            int off = frame->pop().val.i;
            j2me::core::JavaValue arrayVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = -1;

            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* streamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (streamObj->fields.size() > 1) {
                    int streamId = (int)streamObj->fields[1];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    
                    j2me::core::JavaObject* arrayObj = nullptr;
                    size_t arrayLen = 0;
                    
                    if (arrayVal.type == j2me::core::JavaValue::REFERENCE && arrayVal.val.ref != nullptr) {
                        arrayObj = (j2me::core::JavaObject*)arrayVal.val.ref;
                        arrayLen = arrayObj->fields.size();
                    }

                    if (stream && arrayObj != nullptr) {
                        if (off < 0 || len < 0 || off + len > (int)arrayLen) {
                            std::stringstream ss; ss << "[ResourceInputStream.nativeRead([BII)I] ERROR - Invalid parameters: off: " << off << " len: " << len << " arrayLen: " << arrayLen; LOG_ERROR(ss.str().c_str());
                        } else if (len == 0) {
                            result.val.i = 0;
                        } else {
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
                        LOG_ERROR("[ResourceInputStream.nativeRead([BII)I] ERROR - stream or array object not found");
                    }
                } else {
                    LOG_ERROR("[ResourceInputStream.nativeRead([BII)I] ERROR - streamObj has no fields");
                }
            } else {
                LOG_ERROR("[ResourceInputStream.nativeRead([BII)I] ERROR - Invalid this or array object");
            }
            
            frame->push(result);
        }
    );

    // java/io/ResourceInputStream.nativeSkip(J)J
    registry.registerNative("java/io/ResourceInputStream", "nativeSkip", "(J)J", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int64_t n = frame->pop().val.l;
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::LONG;
            result.val.l = 0;
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* streamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (streamObj->fields.size() > 1) {
                    int streamId = (int)streamObj->fields[1];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    
                    if (stream) {
                        result.val.l = stream->skip(n);
                    } else {
                        LOG_ERROR("[ResourceInputStream.nativeSkip(J)J] ERROR - stream not found for id: " + std::to_string(streamId));
                    }
                } else {
                    LOG_ERROR("[ResourceInputStream.nativeSkip(J)J] ERROR - streamObj has no fields");
                }
            } else {
                LOG_ERROR("[ResourceInputStream.nativeSkip(J)J] ERROR - Invalid this object");
            }
            
            frame->push(result);
        }
    );

    // java/io/ResourceInputStream.nativeMark(I)V
    registry.registerNative("java/io/ResourceInputStream", "nativeMark", "(I)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int readlimit = frame->pop().val.i;
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* streamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (streamObj->fields.size() > 1) {
                    int streamId = (int)streamObj->fields[1];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    
                    if (stream) {
                        stream->mark(readlimit);
                    } else {
                        LOG_ERROR("[ResourceInputStream.nativeMark(I)V] ERROR - stream not found for id: " + std::to_string(streamId));
                    }
                } else {
                    LOG_ERROR("[ResourceInputStream.nativeMark(I)V] ERROR - streamObj has no fields");
                }
            } else {
                LOG_ERROR("[ResourceInputStream.nativeMark(I)V] ERROR - Invalid this object");
            }
        }
    );

    // java/io/ResourceInputStream.nativeReset()V
    registry.registerNative("java/io/ResourceInputStream", "nativeReset", "()V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* streamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (streamObj->fields.size() > 1) {
                    int streamId = (int)streamObj->fields[1];
                    auto stream = j2me::core::HeapManager::getInstance().getStream(streamId);
                    
                    if (stream) {
                        stream->reset();
                    } else {
                        LOG_ERROR("[ResourceInputStream.nativeReset()V] ERROR - stream not found for id: " + std::to_string(streamId));
                    }
                } else {
                    LOG_ERROR("[ResourceInputStream.nativeReset()V] ERROR - streamObj has no fields");
                }
            } else {
                LOG_ERROR("[ResourceInputStream.nativeReset()V] ERROR - Invalid this object");
            }
        }
    );

    // java/io/ResourceInputStream.nativeClose()V
    registry.registerNative("java/io/ResourceInputStream", "nativeClose", "()V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* streamObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (streamObj->fields.size() > 1) {
                    int streamId = (int)streamObj->fields[1];
                    j2me::core::HeapManager::getInstance().removeStream(streamId);
                } else {
                    LOG_ERROR("[ResourceInputStream.nativeClose()V] ERROR - streamObj has no fields");
                }
            } else {
                LOG_ERROR("[ResourceInputStream.nativeClose()V] ERROR - Invalid this object");
            }
        }
    );
}

} // namespace natives
} // namespace j2me
