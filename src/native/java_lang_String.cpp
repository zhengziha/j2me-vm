#include "java_lang_String.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include <iostream>
#include <string>
#include <iconv.h>
#include <vector>

namespace j2me {
namespace natives {

j2me::core::JavaObject* createJavaString(j2me::core::Interpreter* interpreter, const std::string& str) {
    if (!interpreter) return nullptr;
    
    auto stringCls = interpreter->resolveClass("java/lang/String");
    if (!stringCls) return nullptr;
    
    auto stringObj = j2me::core::HeapManager::getInstance().allocate(stringCls);
    
    // Create char array for value
    // We try [C first (standard), then [B (fallback)
    auto arrayCls = interpreter->resolveClass("[C");
    if (!arrayCls) arrayCls = interpreter->resolveClass("[B");
    
    if (arrayCls) {
        auto arrayObj = j2me::core::HeapManager::getInstance().allocate(arrayCls);
        arrayObj->fields.resize(str.length());
        for (size_t i = 0; i < str.length(); i++) {
            arrayObj->fields[i] = (uint16_t)(uint8_t)str[i]; 
        }
        
        // Set value field
        auto valueIt = stringCls->fieldOffsets.find("value|[C");
        if (valueIt == stringCls->fieldOffsets.end()) valueIt = stringCls->fieldOffsets.find("value|[B");
        if (valueIt != stringCls->fieldOffsets.end()) {
            stringObj->fields[valueIt->second] = (int64_t)arrayObj;
        }
        
        // Set count field (if exists)
        auto countIt = stringCls->fieldOffsets.find("count|I");
        if (countIt != stringCls->fieldOffsets.end()) {
             stringObj->fields[countIt->second] = str.length();
        }
        
        // Set offset field (if exists)
        auto offsetIt = stringCls->fieldOffsets.find("offset|I");
        if (offsetIt != stringCls->fieldOffsets.end()) {
             stringObj->fields[offsetIt->second] = 0;
        }
    }
    
    return stringObj;
}

std::string getJavaString(j2me::core::JavaObject* strObj) {
    if (!strObj || !strObj->cls) return "";
    
    // Debug: print keys
    
    std::cout << "String fields: ";
    for (const auto& kv : strObj->cls->fieldOffsets) {
        std::cout << kv.first << ", ";
    }
    std::cout << std::endl;
    

    // Try to get from value field
    auto valueIt = strObj->cls->fieldOffsets.find("value|[C");
    if (valueIt == strObj->cls->fieldOffsets.end()) {
        valueIt = strObj->cls->fieldOffsets.find("value|[B");
        if (valueIt == strObj->cls->fieldOffsets.end()) {
             // Fallback to "value" just in case
             valueIt = strObj->cls->fieldOffsets.find("value");
        }
    }
    
    if (valueIt != strObj->cls->fieldOffsets.end()) {
        int64_t arrayRef = strObj->fields[valueIt->second];
        // std::cout << "value arrayRef: " << arrayRef << std::endl;
        if (arrayRef != 0) {
            auto arrayObj = (j2me::core::JavaObject*)arrayRef;
            if (!arrayObj) return "";
            
            // Handle offset and count if they exist
            size_t offset = 0;
            size_t count = arrayObj->fields.size();
            
            auto offsetIt = strObj->cls->fieldOffsets.find("offset|I");
            if (offsetIt == strObj->cls->fieldOffsets.end()) offsetIt = strObj->cls->fieldOffsets.find("offset");

            if (offsetIt != strObj->cls->fieldOffsets.end()) {
                offset = (size_t)strObj->fields[offsetIt->second];
            }
            
            auto countIt = strObj->cls->fieldOffsets.find("count|I");
            if (countIt == strObj->cls->fieldOffsets.end()) countIt = strObj->cls->fieldOffsets.find("count");
            
            if (countIt != strObj->cls->fieldOffsets.end()) {
                count = (size_t)strObj->fields[countIt->second];
            }
            
            std::cout << "getJavaString: arraySize=" << arrayObj->fields.size() 
                      << ", offset=" << offset 
                      << ", count=" << count << std::endl;
            
            // GBK Heuristic Repair:
            // If the string contains only characters <= 0xFF (Latin-1), and has high bytes (> 0x7F),
            // it's likely a GBK byte stream interpreted as ISO-8859-1.
            // We try to convert it back to UTF-8 using GBK decoding.
            if (offset < arrayObj->fields.size()) {
                size_t actualCount = std::min(count, arrayObj->fields.size() - offset);
                
                bool potentialGBK = false;
                bool hasHighByte = false;
                bool hasNonLatin1 = false;
                std::vector<char> rawBytes;
                rawBytes.reserve(actualCount);
                
                for (size_t i = 0; i < actualCount; i++) {
                    uint16_t ch = (uint16_t)arrayObj->fields[offset + i];
                    if (ch > 0xFF) {
                        hasNonLatin1 = true;
                        break;
                    }
                    if (ch > 0x7F) hasHighByte = true;
                    rawBytes.push_back((char)ch);
                }
                
                if (!hasNonLatin1 && hasHighByte && !rawBytes.empty()) {
                     iconv_t cd = iconv_open("UTF-8", "GBK");
                     if (cd != (iconv_t)-1) {
                         std::vector<char> outBuf(rawBytes.size() * 4 + 1); 
                         char* inPtr = rawBytes.data();
                         size_t inBytes = rawBytes.size();
                         char* outPtr = outBuf.data();
                         size_t outBytes = outBuf.size();
                         
                         // We use a temporary copy because iconv modifies pointers
                         if (iconv(cd, &inPtr, &inBytes, &outPtr, &outBytes) != (size_t)-1) {
                             iconv_close(cd);
                             std::string converted(outBuf.data(), outBuf.size() - outBytes);
                             std::cout << "[getJavaString] Repaired GBK double-encoding: " << converted << std::endl;
                             return converted;
                         }
                         iconv_close(cd);
                     }
                }
            }

            std::string res;
            if (offset < arrayObj->fields.size()) {
                size_t actualCount = std::min(count, arrayObj->fields.size() - offset);
                // Reserve enough space (assuming worst case 3 bytes per char for BMP)
                res.reserve(actualCount * 3);
                
                for (size_t i = 0; i < actualCount; i++) {
                    uint16_t ch = (uint16_t)arrayObj->fields[offset + i];
                    
                    if (ch < 0x80) {
                        res += (char)ch;
                    } else if (ch < 0x800) {
                        res += (char)(0xC0 | (ch >> 6));
                        res += (char)(0x80 | (ch & 0x3F));
                    } else {
                        // 3-byte sequence for rest of BMP
                        res += (char)(0xE0 | (ch >> 12));
                        res += (char)(0x80 | ((ch >> 6) & 0x3F));
                        res += (char)(0x80 | (ch & 0x3F));
                    }
                }
            }
            return res;
        }
    }
    
    return "";
}

void registerStringNatives(j2me::core::NativeRegistry& registry) {
    // registry passed as argument

    // java/lang/String.getBytes()[B
    registry.registerNative("java/lang/String", "getBytes", "()[B",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            
            auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
            auto arrayCls = interpreter->resolveClass("[B");
            if (arrayCls) {
                std::string strValue;
                
                if (thisVal.type == j2me::core::JavaValue::REFERENCE) {
                    strValue = thisVal.strVal;
                    std::cout << "[String.getBytes] String value: '" << strValue << "' (length: " << strValue.length() << ")" << std::endl;
                }
                
                auto arrayObj = j2me::core::HeapManager::getInstance().allocate(arrayCls);
                arrayObj->fields.resize(strValue.length());
                for (size_t i = 0; i < strValue.length(); i++) {
                    arrayObj->fields[i] = static_cast<uint8_t>(strValue[i]);
                }
                result.val.ref = arrayObj;
                std::cout << "[String.getBytes] Created byte array with " << arrayObj->fields.size() << " bytes" << std::endl;
            }
            
            frame->push(result);
        }
    );

    // java/lang/String.valueOf(I)Ljava/lang/String;
    registry.registerNative("java/lang/String", "valueOf", "(I)Ljava/lang/String;",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue intVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            
            int value = intVal.val.i;
            auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
            auto stringCls = interpreter->resolveClass("java/lang/String");
            if (stringCls) {
                auto stringObj = j2me::core::HeapManager::getInstance().allocate(stringCls);
                result.val.ref = stringObj;
                result.strVal = std::to_string(value);
            }
            
            frame->push(result);
        }
    );

    // java/lang/String.valueOf(J)Ljava/lang/String;
    registry.registerNative("java/lang/String", "valueOf", "(J)Ljava/lang/String;",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue longVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            
            long value = longVal.val.l;
            auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
            auto stringCls = interpreter->resolveClass("java/lang/String");
            if (stringCls) {
                auto stringObj = j2me::core::HeapManager::getInstance().allocate(stringCls);
                result.val.ref = stringObj;
                result.strVal = std::to_string(value);
            }
            
            frame->push(result);
        }
    );

    // java/lang/String.<init>([B)V
    registry.registerNative("java/lang/String", "<init>", "([B)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue byteArrayVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                auto thisObj = static_cast<j2me::core::JavaObject*>(thisVal.val.ref);
                if (thisObj && thisObj->cls) {
                    if (byteArrayVal.type == j2me::core::JavaValue::REFERENCE && byteArrayVal.val.ref != nullptr) {
                        auto byteArrayObj = static_cast<j2me::core::JavaObject*>(byteArrayVal.val.ref);
                        
                        // Try GBK conversion first
                        bool converted = false;
                        
                        // Collect bytes
                        std::vector<char> inBuf;
                        inBuf.reserve(byteArrayObj->fields.size());
                        for (auto val : byteArrayObj->fields) {
                            inBuf.push_back((char)val);
                        }
                        
                        if (!inBuf.empty()) {
                            std::vector<char> outBuf(inBuf.size() * 2 + 2);
                            iconv_t cd = iconv_open("UTF-16LE", "GBK");
                            
                            if (cd != (iconv_t)-1) {
                                char* inPtr = inBuf.data();
                                size_t inBytes = inBuf.size();
                                char* outPtr = outBuf.data();
                                size_t outBytes = outBuf.size();
                                
                                if (iconv(cd, &inPtr, &inBytes, &outPtr, &outBytes) != (size_t)-1) {
                                    size_t convertedBytes = outBuf.size() - outBytes;
                                    size_t charCount = convertedBytes / 2;
                                    
                                    auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                                    auto charArrayCls = interpreter->resolveClass("[C");
                                    
                                    if (charArrayCls) {
                                        auto charArrayObj = j2me::core::HeapManager::getInstance().allocate(charArrayCls);
                                        charArrayObj->fields.resize(charCount);
                                        
                                        uint16_t* u16Ptr = (uint16_t*)outBuf.data();
                                        for (size_t i = 0; i < charCount; i++) {
                                            charArrayObj->fields[i] = u16Ptr[i];
                                        }
                                        
                                        auto valueIt = thisObj->cls->fieldOffsets.find("value|[C");
                                        if (valueIt == thisObj->cls->fieldOffsets.end()) valueIt = thisObj->cls->fieldOffsets.find("value|[B");
                                        if (valueIt != thisObj->cls->fieldOffsets.end()) {
                                            thisObj->fields[valueIt->second] = reinterpret_cast<intptr_t>(charArrayObj);
                                        }
                                        
                                        auto countIt = thisObj->cls->fieldOffsets.find("count|I");
                                        if (countIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[countIt->second] = charCount;
                                        
                                        auto offsetIt = thisObj->cls->fieldOffsets.find("offset|I");
                                        if (offsetIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[offsetIt->second] = 0;
                                        
                                        converted = true;
                                        std::cout << "[String.<init>([B)] Converted " << inBuf.size() << " GBK bytes to " << charCount << " UTF-16 chars" << std::endl;
                                    }
                                }
                                iconv_close(cd);
                            }
                        }
                        
                        if (!converted) {
                            // Fallback to ISO-8859-1 (original logic)
                            auto valueIt = thisObj->cls->fieldOffsets.find("value|[C");
                            if (valueIt == thisObj->cls->fieldOffsets.end()) valueIt = thisObj->cls->fieldOffsets.find("value|[B");
                            if (valueIt != thisObj->cls->fieldOffsets.end()) {
                                thisObj->fields[valueIt->second] = reinterpret_cast<intptr_t>(byteArrayObj);
                                
                                auto offsetIt = thisObj->cls->fieldOffsets.find("offset|I");
                                if (offsetIt != thisObj->cls->fieldOffsets.end()) {
                                    thisObj->fields[offsetIt->second] = 0;
                                }
                                
                                auto countIt = thisObj->cls->fieldOffsets.find("count|I");
                                if (countIt != thisObj->cls->fieldOffsets.end()) {
                                    thisObj->fields[countIt->second] = byteArrayObj->fields.size();
                                }
                                std::cout << "[String.<init>([B)] Fallback: Created String from byte array (ISO-8859-1) with " << byteArrayObj->fields.size() << " bytes" << std::endl;
                            }
                        }
                    }
                }
            }
        }
    );

     // java/lang/String.<init>([BII)V
    registry.registerNative("java/lang/String", "<init>", "([BII)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue lengthVal = frame->pop();
            j2me::core::JavaValue offsetVal = frame->pop();
            j2me::core::JavaValue byteArrayVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                auto thisObj = static_cast<j2me::core::JavaObject*>(thisVal.val.ref);
                if (thisObj && thisObj->cls) {
                    if (byteArrayVal.type == j2me::core::JavaValue::REFERENCE && byteArrayVal.val.ref != nullptr) {
                        auto byteArrayObj = static_cast<j2me::core::JavaObject*>(byteArrayVal.val.ref);
                        
                        int offset = offsetVal.val.i;
                        int length = lengthVal.val.i;
                        
                        if (offset < 0 || length < 0 || offset + length > (int)byteArrayObj->fields.size()) {
                            throw std::runtime_error("StringIndexOutOfBoundsException");
                        }

                        // Try GBK conversion first
                        bool converted = false;
                        
                        // Collect bytes
                        std::vector<char> inBuf;
                        inBuf.reserve(length);
                        for (int i = 0; i < length; i++) {
                            inBuf.push_back((char)byteArrayObj->fields[offset + i]);
                        }
                        
                        if (!inBuf.empty()) {
                            std::vector<char> outBuf(inBuf.size() * 2 + 2);
                            iconv_t cd = iconv_open("UTF-16LE", "GBK");
                            
                            if (cd != (iconv_t)-1) {
                                char* inPtr = inBuf.data();
                                size_t inBytes = inBuf.size();
                                char* outPtr = outBuf.data();
                                size_t outBytes = outBuf.size();
                                
                                if (iconv(cd, &inPtr, &inBytes, &outPtr, &outBytes) != (size_t)-1) {
                                    size_t convertedBytes = outBuf.size() - outBytes;
                                    size_t charCount = convertedBytes / 2;
                                    
                                    auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                                    auto charArrayCls = interpreter->resolveClass("[C");
                                    
                                    if (charArrayCls) {
                                        auto charArrayObj = j2me::core::HeapManager::getInstance().allocate(charArrayCls);
                                        charArrayObj->fields.resize(charCount);
                                        
                                        uint16_t* u16Ptr = (uint16_t*)outBuf.data();
                                        for (size_t i = 0; i < charCount; i++) {
                                            charArrayObj->fields[i] = u16Ptr[i];
                                        }
                                        
                                        auto valueIt = thisObj->cls->fieldOffsets.find("value|[C");
                                        if (valueIt == thisObj->cls->fieldOffsets.end()) valueIt = thisObj->cls->fieldOffsets.find("value|[B");
                                        if (valueIt != thisObj->cls->fieldOffsets.end()) {
                                            thisObj->fields[valueIt->second] = reinterpret_cast<intptr_t>(charArrayObj);
                                        }
                                        
                                        auto countIt = thisObj->cls->fieldOffsets.find("count|I");
                                        if (countIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[countIt->second] = charCount;
                                        
                                        auto offsetIt = thisObj->cls->fieldOffsets.find("offset|I");
                                        if (offsetIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[offsetIt->second] = 0;
                                        
                                        converted = true;
                                        std::cout << "[String.<init>([BII)] Converted " << inBuf.size() << " GBK bytes to " << charCount << " UTF-16 chars" << std::endl;
                                    }
                                }
                                iconv_close(cd);
                            }
                        }
                        
                        if (!converted) {
                             // Fallback to ISO-8859-1: just copy the byte array slice
                             // Since we can't just point to the original byte array (it might be larger),
                             // we should create a new char array (or byte array) for the slice.
                             
                             auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                             // Try to make a char array to be consistent with modern String
                             auto charArrayCls = interpreter->resolveClass("[C");
                             if (charArrayCls) {
                                 auto charArrayObj = j2me::core::HeapManager::getInstance().allocate(charArrayCls);
                                 charArrayObj->fields.resize(length);
                                 for(int i=0; i<length; i++) {
                                     charArrayObj->fields[i] = (uint16_t)(uint8_t)byteArrayObj->fields[offset+i];
                                 }
                                 
                                 auto valueIt = thisObj->cls->fieldOffsets.find("value|[C");
                                 if (valueIt == thisObj->cls->fieldOffsets.end()) valueIt = thisObj->cls->fieldOffsets.find("value|[B");
                                 if (valueIt != thisObj->cls->fieldOffsets.end()) {
                                     thisObj->fields[valueIt->second] = reinterpret_cast<intptr_t>(charArrayObj);
                                 }
                                 
                                 auto countIt = thisObj->cls->fieldOffsets.find("count|I");
                                 if (countIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[countIt->second] = length;
                                 
                                 auto offsetIt = thisObj->cls->fieldOffsets.find("offset|I");
                                 if (offsetIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[offsetIt->second] = 0;
                                 
                                 std::cout << "[String.<init>([BII)] Fallback: Created String from byte slice (ISO-8859-1) with " << length << " chars" << std::endl;
                             }
                        }
                    }
                }
            }
        }
    );

    // java/lang/String.<init>([BLjava/lang/String;)V
    registry.registerNative("java/lang/String", "<init>", "([BLjava/lang/String;)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue charsetVal = frame->pop();
            j2me::core::JavaValue byteArrayVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            std::string charsetName = "GBK"; // Default to GBK if not provided or empty
            if (charsetVal.type == j2me::core::JavaValue::REFERENCE && charsetVal.val.ref != nullptr) {
                auto charsetObj = static_cast<j2me::core::JavaObject*>(charsetVal.val.ref);
                std::string s = getJavaString(charsetObj);
                if (!s.empty()) charsetName = s;
            }

            // Map common Java charset names to iconv names
            if (charsetName == "UTF8") charsetName = "UTF-8";
            // Add more mappings if needed

            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                auto thisObj = static_cast<j2me::core::JavaObject*>(thisVal.val.ref);
                if (thisObj && thisObj->cls) {
                    if (byteArrayVal.type == j2me::core::JavaValue::REFERENCE && byteArrayVal.val.ref != nullptr) {
                        auto byteArrayObj = static_cast<j2me::core::JavaObject*>(byteArrayVal.val.ref);
                        
                        bool converted = false;
                        
                        std::vector<char> inBuf;
                        inBuf.reserve(byteArrayObj->fields.size());
                        for (auto val : byteArrayObj->fields) {
                            inBuf.push_back((char)val);
                        }
                        
                        if (!inBuf.empty()) {
                            std::vector<char> outBuf(inBuf.size() * 2 + 2);
                            iconv_t cd = iconv_open("UTF-16LE", charsetName.c_str());
                            
                            if (cd != (iconv_t)-1) {
                                char* inPtr = inBuf.data();
                                size_t inBytes = inBuf.size();
                                char* outPtr = outBuf.data();
                                size_t outBytes = outBuf.size();
                                
                                if (iconv(cd, &inPtr, &inBytes, &outPtr, &outBytes) != (size_t)-1) {
                                    size_t convertedBytes = outBuf.size() - outBytes;
                                    size_t charCount = convertedBytes / 2;
                                    
                                    auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                                    auto charArrayCls = interpreter->resolveClass("[C");
                                    
                                    if (charArrayCls) {
                                        auto charArrayObj = j2me::core::HeapManager::getInstance().allocate(charArrayCls);
                                        charArrayObj->fields.resize(charCount);
                                        
                                        uint16_t* u16Ptr = (uint16_t*)outBuf.data();
                                        for (size_t i = 0; i < charCount; i++) {
                                            charArrayObj->fields[i] = u16Ptr[i];
                                        }
                                        
                                        auto valueIt = thisObj->cls->fieldOffsets.find("value|[C");
                                        if (valueIt == thisObj->cls->fieldOffsets.end()) valueIt = thisObj->cls->fieldOffsets.find("value|[B");
                                        if (valueIt != thisObj->cls->fieldOffsets.end()) {
                                            thisObj->fields[valueIt->second] = reinterpret_cast<intptr_t>(charArrayObj);
                                        }
                                        
                                        auto countIt = thisObj->cls->fieldOffsets.find("count|I");
                                        if (countIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[countIt->second] = charCount;
                                        
                                        auto offsetIt = thisObj->cls->fieldOffsets.find("offset|I");
                                        if (offsetIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[offsetIt->second] = 0;
                                        
                                        converted = true;
                                        std::cout << "[String.<init>([BLString;)] Converted " << inBuf.size() << " bytes using " << charsetName << " to " << charCount << " UTF-16 chars" << std::endl;
                                    }
                                }
                                iconv_close(cd);
                            } else {
                                std::cerr << "[String.<init>] iconv_open failed for charset: " << charsetName << std::endl;
                            }
                        }
                        
                        if (!converted) {
                             // Fallback to ISO-8859-1
                             auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                             auto charArrayCls = interpreter->resolveClass("[C");
                             if (charArrayCls) {
                                 auto charArrayObj = j2me::core::HeapManager::getInstance().allocate(charArrayCls);
                                 charArrayObj->fields.resize(byteArrayObj->fields.size());
                                 for(size_t i=0; i<byteArrayObj->fields.size(); i++) {
                                     charArrayObj->fields[i] = (uint16_t)(uint8_t)byteArrayObj->fields[i];
                                 }
                                 
                                 auto valueIt = thisObj->cls->fieldOffsets.find("value|[C");
                                 if (valueIt == thisObj->cls->fieldOffsets.end()) valueIt = thisObj->cls->fieldOffsets.find("value|[B");
                                 if (valueIt != thisObj->cls->fieldOffsets.end()) {
                                     thisObj->fields[valueIt->second] = reinterpret_cast<intptr_t>(charArrayObj);
                                 }
                                 
                                 auto countIt = thisObj->cls->fieldOffsets.find("count|I");
                                 if (countIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[countIt->second] = byteArrayObj->fields.size();
                                 
                                 auto offsetIt = thisObj->cls->fieldOffsets.find("offset|I");
                                 if (offsetIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[offsetIt->second] = 0;
                                 
                                 std::cout << "[String.<init>([BLString;)] Fallback: Created String from byte array (ISO-8859-1) with " << byteArrayObj->fields.size() << " bytes" << std::endl;
                             }
                        }
                    }
                }
            }
        }
    );

    // java/lang/String.<init>([CII)V
    registry.registerNative("java/lang/String", "<init>", "([CII)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue lengthVal = frame->pop();
            j2me::core::JavaValue offsetVal = frame->pop();
            j2me::core::JavaValue charArrayVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                auto thisObj = static_cast<j2me::core::JavaObject*>(thisVal.val.ref);
                if (thisObj && thisObj->cls) {
                    if (charArrayVal.type == j2me::core::JavaValue::REFERENCE && charArrayVal.val.ref != nullptr) {
                        auto charArrayObj = static_cast<j2me::core::JavaObject*>(charArrayVal.val.ref);
                        
                        int offset = offsetVal.val.i;
                        int length = lengthVal.val.i;
                        
                        if (offset < 0 || length < 0 || offset + length > (int)charArrayObj->fields.size()) {
                            throw std::runtime_error("StringIndexOutOfBoundsException");
                        }
                        
                        bool converted = false;
                        
                        // GBK Heuristic: Check if chars are actually bytes (<= 0xFF) and contain GBK
                        bool allLatin1 = true;
                        bool hasHigh = false;
                        std::vector<char> inBuf;
                        inBuf.reserve(length);
                        
                        for(int i=0; i<length; i++) {
                            uint16_t c = charArrayObj->fields[offset+i];
                            if (c > 0xFF) {
                                allLatin1 = false;
                                break;
                            }
                            if (c > 0x7F) hasHigh = true;
                            inBuf.push_back((char)c);
                        }
                        
                        if (allLatin1 && hasHigh && !inBuf.empty()) {
                            std::vector<char> outBuf(inBuf.size() * 2 + 2);
                            iconv_t cd = iconv_open("UTF-16LE", "GBK");
                            
                            if (cd != (iconv_t)-1) {
                                char* inPtr = inBuf.data();
                                size_t inBytes = inBuf.size();
                                char* outPtr = outBuf.data();
                                size_t outBytes = outBuf.size();
                                
                                if (iconv(cd, &inPtr, &inBytes, &outPtr, &outBytes) != (size_t)-1) {
                                    size_t convertedBytes = outBuf.size() - outBytes;
                                    size_t charCount = convertedBytes / 2;
                                    
                                    auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                                    auto newCharArrayCls = interpreter->resolveClass("[C");
                                    
                                    if (newCharArrayCls) {
                                        auto newCharArrayObj = j2me::core::HeapManager::getInstance().allocate(newCharArrayCls);
                                        newCharArrayObj->fields.resize(charCount);
                                        
                                        uint16_t* u16Ptr = (uint16_t*)outBuf.data();
                                        for (size_t i = 0; i < charCount; i++) {
                                            newCharArrayObj->fields[i] = u16Ptr[i];
                                        }
                                        
                                        auto valueIt = thisObj->cls->fieldOffsets.find("value|[C");
                                        if (valueIt == thisObj->cls->fieldOffsets.end()) valueIt = thisObj->cls->fieldOffsets.find("value|[B");
                                        if (valueIt != thisObj->cls->fieldOffsets.end()) {
                                            thisObj->fields[valueIt->second] = reinterpret_cast<intptr_t>(newCharArrayObj);
                                        }
                                        
                                        auto countIt = thisObj->cls->fieldOffsets.find("count|I");
                                        if (countIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[countIt->second] = charCount;
                                        
                                        auto offsetIt = thisObj->cls->fieldOffsets.find("offset|I");
                                        if (offsetIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[offsetIt->second] = 0;
                                        
                                        converted = true;
                                        std::cout << "[String.<init>([CII)] Repaired GBK chars: converted " << length << " Latin1 chars to " << charCount << " UTF-16 chars" << std::endl;
                                    }
                                }
                                iconv_close(cd);
                            }
                        }
                        
                        if (!converted) {
                             // Normal copy
                             auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                             auto newCharArrayCls = interpreter->resolveClass("[C");
                             if (newCharArrayCls) {
                                 auto newCharArrayObj = j2me::core::HeapManager::getInstance().allocate(newCharArrayCls);
                                 newCharArrayObj->fields.resize(length);
                                 for(int i=0; i<length; i++) {
                                     newCharArrayObj->fields[i] = charArrayObj->fields[offset+i];
                                 }
                                 
                                 auto valueIt = thisObj->cls->fieldOffsets.find("value|[C");
                                 if (valueIt == thisObj->cls->fieldOffsets.end()) valueIt = thisObj->cls->fieldOffsets.find("value|[B");
                                 if (valueIt != thisObj->cls->fieldOffsets.end()) {
                                     thisObj->fields[valueIt->second] = reinterpret_cast<intptr_t>(newCharArrayObj);
                                 }
                                 
                                 auto countIt = thisObj->cls->fieldOffsets.find("count|I");
                                 if (countIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[countIt->second] = length;
                                 
                                 auto offsetIt = thisObj->cls->fieldOffsets.find("offset|I");
                                 if (offsetIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[offsetIt->second] = 0;
                                 
                                 // std::cout << "[String.<init>([CII)] Standard copy" << std::endl;
                             }
                        }
                    }
                }
            }
        }
    );

    // java/lang/String.<init>([C)V
    registry.registerNative("java/lang/String", "<init>", "([C)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue charArrayVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                auto thisObj = static_cast<j2me::core::JavaObject*>(thisVal.val.ref);
                if (thisObj && thisObj->cls) {
                    if (charArrayVal.type == j2me::core::JavaValue::REFERENCE && charArrayVal.val.ref != nullptr) {
                        auto charArrayObj = static_cast<j2me::core::JavaObject*>(charArrayVal.val.ref);
                        
                        int length = charArrayObj->fields.size();
                        bool converted = false;
                        
                        // GBK Heuristic
                        bool allLatin1 = true;
                        bool hasHigh = false;
                        std::vector<char> inBuf;
                        inBuf.reserve(length);
                        
                        for(int i=0; i<length; i++) {
                            uint16_t c = charArrayObj->fields[i];
                            if (c > 0xFF) {
                                allLatin1 = false;
                                break;
                            }
                            if (c > 0x7F) hasHigh = true;
                            inBuf.push_back((char)c);
                        }
                        
                        if (allLatin1 && hasHigh && !inBuf.empty()) {
                            std::vector<char> outBuf(inBuf.size() * 2 + 2);
                            iconv_t cd = iconv_open("UTF-16LE", "GBK");
                            
                            if (cd != (iconv_t)-1) {
                                char* inPtr = inBuf.data();
                                size_t inBytes = inBuf.size();
                                char* outPtr = outBuf.data();
                                size_t outBytes = outBuf.size();
                                
                                if (iconv(cd, &inPtr, &inBytes, &outPtr, &outBytes) != (size_t)-1) {
                                    size_t convertedBytes = outBuf.size() - outBytes;
                                    size_t charCount = convertedBytes / 2;
                                    
                                    auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                                    auto newCharArrayCls = interpreter->resolveClass("[C");
                                    
                                    if (newCharArrayCls) {
                                        auto newCharArrayObj = j2me::core::HeapManager::getInstance().allocate(newCharArrayCls);
                                        newCharArrayObj->fields.resize(charCount);
                                        
                                        uint16_t* u16Ptr = (uint16_t*)outBuf.data();
                                        for (size_t i = 0; i < charCount; i++) {
                                            newCharArrayObj->fields[i] = u16Ptr[i];
                                        }
                                        
                                        auto valueIt = thisObj->cls->fieldOffsets.find("value|[C");
                                        if (valueIt == thisObj->cls->fieldOffsets.end()) valueIt = thisObj->cls->fieldOffsets.find("value|[B");
                                        if (valueIt != thisObj->cls->fieldOffsets.end()) {
                                            thisObj->fields[valueIt->second] = reinterpret_cast<intptr_t>(newCharArrayObj);
                                        }
                                        
                                        auto countIt = thisObj->cls->fieldOffsets.find("count|I");
                                        if (countIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[countIt->second] = charCount;
                                        
                                        auto offsetIt = thisObj->cls->fieldOffsets.find("offset|I");
                                        if (offsetIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[offsetIt->second] = 0;
                                        
                                        converted = true;
                                        std::cout << "[String.<init>([C)] Repaired GBK chars: converted " << length << " Latin1 chars to " << charCount << " UTF-16 chars" << std::endl;
                                    }
                                }
                                iconv_close(cd);
                            }
                        }
                        
                        if (!converted) {
                             // Normal copy
                             auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                             auto newCharArrayCls = interpreter->resolveClass("[C");
                             if (newCharArrayCls) {
                                 auto newCharArrayObj = j2me::core::HeapManager::getInstance().allocate(newCharArrayCls);
                                 newCharArrayObj->fields.resize(length);
                                 for(int i=0; i<length; i++) {
                                     newCharArrayObj->fields[i] = charArrayObj->fields[i];
                                 }
                                 
                                 auto valueIt = thisObj->cls->fieldOffsets.find("value|[C");
                                 if (valueIt == thisObj->cls->fieldOffsets.end()) valueIt = thisObj->cls->fieldOffsets.find("value|[B");
                                 if (valueIt != thisObj->cls->fieldOffsets.end()) {
                                     thisObj->fields[valueIt->second] = reinterpret_cast<intptr_t>(newCharArrayObj);
                                 }
                                 
                                 auto countIt = thisObj->cls->fieldOffsets.find("count|I");
                                 if (countIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[countIt->second] = length;
                                 
                                 auto offsetIt = thisObj->cls->fieldOffsets.find("offset|I");
                                 if (offsetIt != thisObj->cls->fieldOffsets.end()) thisObj->fields[offsetIt->second] = 0;
                                 
                                 // std::cout << "[String.<init>([C)] Standard copy" << std::endl;
                             }
                        }
                    }
                }
            }
        }
    );
}

}
}
