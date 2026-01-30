#include "NativeRegistry.hpp"
#include "RuntimeTypes.hpp"
#include "HeapManager.hpp"
#include "Interpreter.hpp"
#include "../native/java_lang_Class.hpp"
#include "../native/java_lang_Thread.hpp"
#include "../native/java_lang_String.hpp"
#include "../native/java_lang_StringBuffer.hpp"
#include "../native/java_lang_System.hpp"
#include "../native/java_io_DataOutputStream.hpp"
#include "../native/java_lang_Object.hpp"
#include "../native/java_lang_Float.hpp"
#include "../native/java_lang_Double.hpp"
#include "../native/java_lang_Math.hpp"
#include "../native/java_io_InputStream.hpp"
#include "../native/java_io_PrintStream.hpp"
#include "../native/java_io_ByteArrayInputStream.hpp"
#include "../native/java_io_ResourceInputStream.hpp"
#include "../native/javax_microedition_lcdui_Graphics.hpp"
#include "../native/javax_microedition_lcdui_Display.hpp"
#include "../native/javax_microedition_lcdui_Image.hpp"
#include "../native/javax_microedition_lcdui_Font.hpp"
#include "../native/javax_microedition_lcdui_game_GameCanvas.hpp"
#include "../native/javax_microedition_lcdui_game_Sprite.hpp"
#include "../native/javax_microedition_lcdui_game_TiledLayer.hpp"
#include "../native/javax_microedition_media_Manager.hpp"
#include "../native/j2me_media_AudioPlayer.hpp"
#include "../native/javax_microedition_rms_RecordStore.hpp"
#include "../native/java_util_Timer.hpp"
#include "../native/javax_microedition_io_Connector.hpp"
#include "../native/java_io_RandomAccessFile.hpp"
#include "../native/java_lang_Class.hpp"
#include <iostream>

namespace j2me {
namespace natives {
    // Forward declaration to avoid header dependency cycle or just to fix linker
    // void registerTimerNatives(j2me::core::NativeRegistry& registry); // It is in header
}

namespace core {

NativeRegistry::NativeRegistry() {
    // Register natives from other files
    j2me::natives::registerClassNatives(*this);
    j2me::natives::registerThreadNatives(*this);
    j2me::natives::registerStringNatives(*this);
    j2me::natives::registerStringBufferNatives(*this);
    j2me::natives::registerSystemNatives(*this);
    j2me::natives::registerObjectNatives(*this);
    j2me::natives::registerFloatNatives(*this);
    j2me::natives::registerDoubleNatives(*this);
    j2me::natives::registerMathNatives(*this);
    j2me::natives::registerInputStreamNatives(*this);
    j2me::natives::registerPrintStreamNatives(*this);
    j2me::natives::registerDisplayNatives(*this);
    j2me::natives::registerGraphicsNatives(*this);
    j2me::natives::registerImageNatives(*this);
    j2me::natives::registerFontNatives(*this);
    j2me::natives::registerGameCanvasNatives(*this);
    j2me::natives::registerSpriteNatives(*this);
    j2me::natives::registerTiledLayerNatives(*this);
    j2me::natives::registerMediaNatives(*this);
    j2me::natives::registerAudioPlayerNatives(*this);
    // j2me::natives::registerAudioVolumeControlNatives(*this);
    j2me::natives::registerRecordStoreNatives(*this);
    j2me::natives::registerTimerNatives(*this);
    j2me::natives::registerConnectorNatives(*this);
    j2me::natives::registerRandomAccessFileNatives(*this);
    j2me::natives::registerDataOutputStreamNatives(*this);
    j2me::natives::registerByteArrayInputStreamNatives(*this);
    j2me::natives::registerResourceInputStreamNatives(*this);

    // java/lang/AbstractStringBuilder.<init>(I)V
    registerNative("java/lang/AbstractStringBuilder", "<init>", "(I)V", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue capacityVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        if (thisObj) {
            std::string* str = new std::string();
            if (thisObj->fields.size() < 1) thisObj->fields.resize(1);
            thisObj->fields[0] = (int64_t)str;
        }
    });

    // java/lang/AbstractStringBuilder.append(Ljava/lang/String;)Ljava/lang/AbstractStringBuilder;
    registerNative("java/lang/AbstractStringBuilder", "append", "(Ljava/lang/String;)Ljava/lang/AbstractStringBuilder;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue strVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                if (strVal.type == JavaValue::REFERENCE && strVal.val.ref != nullptr) {
                     JavaObject* sObj = (JavaObject*)strVal.val.ref;
                     std::string s = j2me::natives::getJavaString(sObj);
                     str->append(s);
                } else if (strVal.type == JavaValue::REFERENCE && strVal.val.ref == nullptr) {
                     str->append("null");
                }
            }
        }
        frame->push(thisVal);
    });

    // java/lang/AbstractStringBuilder.append(I)Ljava/lang/AbstractStringBuilder;
    registerNative("java/lang/AbstractStringBuilder", "append", "(I)Ljava/lang/AbstractStringBuilder;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue intVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                str->append(std::to_string(intVal.val.i));
            }
        }
        frame->push(thisVal);
    });

    // java/lang/AbstractStringBuilder.append(Ljava/lang/Object;)Ljava/lang/AbstractStringBuilder;
    registerNative("java/lang/AbstractStringBuilder", "append", "(Ljava/lang/Object;)Ljava/lang/AbstractStringBuilder;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue objVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                std::string objStr = "null";
                if (objVal.type == JavaValue::REFERENCE && objVal.val.ref != nullptr) {
                    JavaObject* obj = (JavaObject*)objVal.val.ref;
                    if (obj && obj->cls && obj->cls->name == "java/lang/String") {
                        objStr = j2me::natives::getJavaString(obj);
                    } else {
                        objStr = "Object";
                    }
                }
                str->append(objStr);
            }
        }
        frame->push(thisVal);
    });

    // java/lang/AbstractStringBuilder.append(Ljava/lang/CharSequence;)Ljava/lang/AbstractStringBuilder;
    registerNative("java/lang/AbstractStringBuilder", "append", "(Ljava/lang/CharSequence;)Ljava/lang/AbstractStringBuilder;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue csVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str && csVal.type == JavaValue::REFERENCE && csVal.val.ref != nullptr) {
                JavaObject* cs = (JavaObject*)csVal.val.ref;
                if (cs && cs->cls && cs->cls->name == "java/lang/String") {
                    str->append(j2me::natives::getJavaString(cs));
                }
            }
        }
        frame->push(thisVal);
    });

    // java/lang/AbstractStringBuilder.append([C)Ljava/lang/AbstractStringBuilder;
    registerNative("java/lang/AbstractStringBuilder", "append", "([C)Ljava/lang/AbstractStringBuilder;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue arrVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str && arrVal.type == JavaValue::REFERENCE && arrVal.val.ref != nullptr) {
                JavaObject* arr = (JavaObject*)arrVal.val.ref;
                if (arr && !arr->fields.empty()) {
                    size_t len = arr->fields.size();
                    for (size_t i = 0; i < len; i++) {
                        char c = (char)(arr->fields[i] & 0xFF);
                        str->append(1, c);
                    }
                }
            }
        }
        frame->push(thisVal);
    });

    // java/lang/AbstractStringBuilder.length()I
    registerNative("java/lang/AbstractStringBuilder", "length", "()I", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue thisVal = frame->pop();
        JavaValue result;
        result.type = JavaValue::INT;
        result.val.i = 0;

        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                result.val.i = (int32_t)str->length();
            }
        }
        frame->push(result);
    });

    // java/lang/AbstractStringBuilder.capacity()I
    registerNative("java/lang/AbstractStringBuilder", "capacity", "()I", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue thisVal = frame->pop();
        JavaValue result;
        result.type = JavaValue::INT;
        result.val.i = 2147483647; // Return max int for simplicity
        
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                result.val.i = (int32_t)str->capacity();
            }
        }
        frame->push(result);
    });

    // java/lang/AbstractStringBuilder.toString()Ljava/lang/String;
    registerNative("java/lang/AbstractStringBuilder", "toString", "()Ljava/lang/String;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue thisVal = frame->pop();
        JavaValue result;
        result.type = JavaValue::REFERENCE;
        result.val.ref = nullptr;
        
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                auto interpreter = NativeRegistry::getInstance().getInterpreter();
                if (interpreter) {
                    result.val.ref = j2me::natives::createJavaString(interpreter, *str);
                }
            }
        }
        frame->push(result);
    });

    // java/lang/StringBuilder.initNative()V
    registerNative("java/lang/StringBuilder", "initNative", "()V", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        if (thisObj) {
            // std::cerr << "DEBUG: StringBuilder.initNative() called, fields.size() = " << thisObj->fields.size() << std::endl;
            std::string* str = new std::string();
            // Store pointer in first field (ensure size)
            if (thisObj->fields.size() < 1) thisObj->fields.resize(1);
            thisObj->fields[0] = (int64_t)str;
            // std::cerr << "DEBUG: StringBuilder.initNative() completed, fields.size() = " << thisObj->fields.size() << std::endl;
        } else {
            std::cerr << "DEBUG: StringBuilder.initNative() called with null thisObj!" << std::endl;
        }
    });

    // java/lang/StringBuilder.append(Ljava/lang/String;)Ljava/lang/StringBuilder;
    registerNative("java/lang/StringBuilder", "append", "(Ljava/lang/String;)Ljava/lang/StringBuilder;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue strVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                if (strVal.type == JavaValue::REFERENCE && strVal.val.ref != nullptr) {
                     JavaObject* sObj = (JavaObject*)strVal.val.ref;
                     std::string s = j2me::natives::getJavaString(sObj);
                     str->append(s);
                } else if (strVal.type == JavaValue::REFERENCE && strVal.val.ref == nullptr) {
                     str->append("null");
                }
            }
        }
        frame->push(thisVal); // Return this
    });

    // java/lang/StringBuilder.append(I)Ljava/lang/StringBuilder;
    registerNative("java/lang/StringBuilder", "append", "(I)Ljava/lang/StringBuilder;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue intVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                str->append(std::to_string(intVal.val.i));
            }
        }
        frame->push(thisVal);
    });

    // java/lang/StringBuilder.append(J)Ljava/lang/StringBuilder;
    registerNative("java/lang/StringBuilder", "append", "(J)Ljava/lang/StringBuilder;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue longVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                str->append(std::to_string(longVal.val.l));
            }
        }
        frame->push(thisVal);
    });

    // java/lang/StringBuilder.append(C)Ljava/lang/StringBuilder;
    registerNative("java/lang/StringBuilder", "append", "(C)Ljava/lang/StringBuilder;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue charVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                str->push_back((char)charVal.val.i);
            }
        }
        frame->push(thisVal);
    });

    // java/lang/StringBuilder.append(Z)Ljava/lang/StringBuilder;
    registerNative("java/lang/StringBuilder", "append", "(Z)Ljava/lang/StringBuilder;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue boolVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                str->append(boolVal.val.i ? "true" : "false");
            }
        }
        frame->push(thisVal);
    });

    // java/lang/StringBuilder.append(F)Ljava/lang/StringBuilder;
    registerNative("java/lang/StringBuilder", "append", "(F)Ljava/lang/StringBuilder;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue floatVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                str->append(std::to_string(floatVal.val.f));
            }
        }
        frame->push(thisVal);
    });

    // java/lang/StringBuilder.append(D)Ljava/lang/StringBuilder;
    registerNative("java/lang/StringBuilder", "append", "(D)Ljava/lang/StringBuilder;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue doubleVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                str->append(std::to_string(doubleVal.val.d));
            }
        }
        frame->push(thisVal);
    });

    // java/lang/StringBuilder.append(Ljava/lang/Object;)Ljava/lang/StringBuilder;
    registerNative("java/lang/StringBuilder", "append", "(Ljava/lang/Object;)Ljava/lang/StringBuilder;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue objVal = frame->pop();
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                if (objVal.val.ref == nullptr) {
                    str->append("null");
                } else {
                    // Try to use strVal hack if present (e.g. from String)
                    if (objVal.type == JavaValue::REFERENCE && !objVal.strVal.empty()) {
                        str->append(objVal.strVal);
                    } else {
                         // Default object toString
                         JavaObject* obj = (JavaObject*)objVal.val.ref;
                         if (obj) {
                             std::string name = (obj->cls) ? obj->cls->name : "Object";
                             str->append(name + "@" + std::to_string((intptr_t)obj));
                         } else {
                             str->append("null");
                         }
                    }
                }
            }
        }
        frame->push(thisVal);
    });

    // java/lang/StringBuilder.toString()Ljava/lang/String;
    registerNative("java/lang/StringBuilder", "toString", "()Ljava/lang/String;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        JavaValue thisVal = frame->pop();
        JavaObject* thisObj = (JavaObject*)thisVal.val.ref;
        
        JavaValue ret;
        ret.type = JavaValue::REFERENCE;
        ret.val.ref = nullptr;
        
        if (thisObj && !thisObj->fields.empty()) {
            std::string* str = (std::string*)thisObj->fields[0];
            if (str) {
                auto interpreter = NativeRegistry::getInstance().getInterpreter();
                auto stringCls = interpreter->resolveClass("java/lang/String");
                if (stringCls) {
                    auto stringObj = HeapManager::getInstance().allocate(stringCls);
                    
                    auto arrayCls = interpreter->resolveClass("[C");
                    if (arrayCls) {
                        auto arrayObj = HeapManager::getInstance().allocate(arrayCls);
                        arrayObj->fields.resize(str->length());
                        for (size_t i = 0; i < str->length(); i++) {
                            arrayObj->fields[i] = (*str)[i];
                        }
                        
                        auto valueIt = stringCls->fieldOffsets.find("value|[C");
                        if (valueIt == stringCls->fieldOffsets.end()) {
                            valueIt = stringCls->fieldOffsets.find("value");
                        }
                        if (valueIt != stringCls->fieldOffsets.end()) {
                            stringObj->fields[valueIt->second] = (int64_t)arrayObj;
                        }
                        
                        auto offsetIt = stringCls->fieldOffsets.find("offset|I");
                        if (offsetIt == stringCls->fieldOffsets.end()) {
                            offsetIt = stringCls->fieldOffsets.find("offset");
                        }
                        if (offsetIt != stringCls->fieldOffsets.end()) {
                            stringObj->fields[offsetIt->second] = 0;
                        }
                        
                        auto countIt = stringCls->fieldOffsets.find("count|I");
                        if (countIt == stringCls->fieldOffsets.end()) {
                            countIt = stringCls->fieldOffsets.find("count");
                        }
                        if (countIt != stringCls->fieldOffsets.end()) {
                            stringObj->fields[countIt->second] = str->length();
                        }
                    }
                    
                    ret.val.ref = stringObj;
                    ret.strVal = *str;
                }
            }
        }
        frame->push(ret);
    });

    // java/lang/String.valueOf(I)Ljava/lang/String;
    registerNative("java/lang/String", "valueOf", "(I)Ljava/lang/String;", [](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame) {
        int val = frame->pop().val.i;
        JavaValue ret;
        ret.type = JavaValue::REFERENCE;
        ret.strVal = std::to_string(val);
        frame->push(ret);
    });
}

void NativeRegistry::registerNative(const std::string& className, const std::string& methodName, const std::string& descriptor, NativeFunction func) {
    std::string key = makeKey(className, methodName, descriptor);
    // std::cout << "[NativeRegistry] Registering: " << key << std::endl;
    registry[key] = func;
}

NativeFunction NativeRegistry::getNative(const std::string& className, const std::string& methodName, const std::string& descriptor) {
    std::string key = makeKey(className, methodName, descriptor);
    auto it = registry.find(key);
    if (it != registry.end()) {
        if (!it->second) {
             std::cerr << "[NativeRegistry] FATAL: Found key but function is empty: " << key << std::endl;
        }
        return it->second;
    }
    // std::cerr << "[NativeRegistry] Native not found: " << key << std::endl;
    return nullptr;
}

std::string NativeRegistry::makeKey(const std::string& className, const std::string& methodName, const std::string& descriptor) {
    return className + "." + methodName + descriptor;
}

} // namespace core
} // namespace j2me
