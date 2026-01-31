#include "../Interpreter.hpp"
#include "../Opcodes.hpp"
#include "../HeapManager.hpp"
#include "../ClassFile.hpp"
#include "../Logger.hpp"

namespace j2me {
namespace core {

// Helper to convert UTF-8 (Modified UTF-8) to UTF-16
static std::vector<uint16_t> utf8ToUtf16(const std::string& utf8) {
    std::vector<uint16_t> utf16;
    size_t i = 0;
    while (i < utf8.length()) {
        uint8_t c = utf8[i++];
        if (c < 0x80) {
            utf16.push_back(c);
        } else if ((c & 0xE0) == 0xC0) {
            if (i < utf8.length()) {
                uint8_t c2 = utf8[i++];
                utf16.push_back(((c & 0x1F) << 6) | (c2 & 0x3F));
            }
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 1 < utf8.length()) {
                uint8_t c2 = utf8[i++];
                uint8_t c3 = utf8[i++];
                utf16.push_back(((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F));
            }
        } else {
             // Skip invalid or 4-byte sequences for now (Java strings are mostly BMP)
        }
    }
    return utf16;
}

void Interpreter::initConstants() {
    // Default handler (can be kept in initInstructionTable or here if we want)
    // Here we only set constants.

    instructionTable[OP_NOP] = instructionTable[OP_ACONST_NULL] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            JavaValue v; v.type = JavaValue::REFERENCE; v.val.ref = nullptr;
            frame->push(v);
            break;
        } while(0);
        return true;
    };

    // Constants - Int
    instructionTable[OP_ICONST_M1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader&, uint8_t) -> bool { frame->push(JavaValue{JavaValue::INT, {.i = -1}}); return true; };
    instructionTable[OP_ICONST_0] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader&, uint8_t) -> bool { frame->push(JavaValue{JavaValue::INT, {.i = 0}}); return true; };
    instructionTable[OP_ICONST_1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader&, uint8_t) -> bool { frame->push(JavaValue{JavaValue::INT, {.i = 1}}); return true; };
    instructionTable[OP_ICONST_2] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader&, uint8_t) -> bool { frame->push(JavaValue{JavaValue::INT, {.i = 2}}); return true; };
    instructionTable[OP_ICONST_3] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader&, uint8_t) -> bool { frame->push(JavaValue{JavaValue::INT, {.i = 3}}); return true; };
    instructionTable[OP_ICONST_4] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader&, uint8_t) -> bool { frame->push(JavaValue{JavaValue::INT, {.i = 4}}); return true; };
    instructionTable[OP_ICONST_5] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader&, uint8_t) -> bool { frame->push(JavaValue{JavaValue::INT, {.i = 5}}); return true; };

    // Constants - Long
    instructionTable[OP_LCONST_0] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader&, uint8_t) -> bool { frame->push(JavaValue{JavaValue::LONG, {.l = 0}}); return true; };
    instructionTable[OP_LCONST_1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader&, uint8_t) -> bool { frame->push(JavaValue{JavaValue::LONG, {.l = 1}}); return true; };

    // Constants - Float
    instructionTable[OP_FCONST_0] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader&, uint8_t) -> bool { JavaValue v; v.type = JavaValue::FLOAT; v.val.f = 0.0f; frame->push(v); return true; };
    instructionTable[OP_FCONST_1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader&, uint8_t) -> bool { JavaValue v; v.type = JavaValue::FLOAT; v.val.f = 1.0f; frame->push(v); return true; };
    instructionTable[OP_FCONST_2] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader&, uint8_t) -> bool { JavaValue v; v.type = JavaValue::FLOAT; v.val.f = 2.0f; frame->push(v); return true; };

    // Constants - Double
    instructionTable[OP_DCONST_0] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader&, uint8_t) -> bool { JavaValue v; v.type = JavaValue::DOUBLE; v.val.d = 0.0; frame->push(v); return true; };
    instructionTable[OP_DCONST_1] = [this](std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame> frame, util::DataReader&, uint8_t) -> bool { JavaValue v; v.type = JavaValue::DOUBLE; v.val.d = 1.0; frame->push(v); return true; };

    // BIPUSH
    instructionTable[OP_BIPUSH] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int8_t byteVal = (int8_t)codeReader.readU1();
            JavaValue v; v.type = JavaValue::INT; v.val.i = byteVal;
            frame->push(v);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_SIPUSH] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            int16_t shortVal = (int16_t)codeReader.readU2();
            JavaValue v; v.type = JavaValue::INT; v.val.i = shortVal;
            frame->push(v);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LDC] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            uint8_t index = codeReader.readU1();
            auto constant = frame->classFile->constant_pool[index];
            if (auto utf8 = std::dynamic_pointer_cast<ConstantString>(constant)) {
                auto strVal = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[utf8->string_index]);
                JavaValue val;
                val.type = JavaValue::REFERENCE; 
                val.strVal = strVal->bytes;
                
                auto stringCls = resolveClass("java/lang/String");
                if (stringCls) {
                    auto stringObj = HeapManager::getInstance().allocate(stringCls);
                    
                    auto valueIt = stringCls->fieldOffsets.find("value|[C");
                    if (valueIt == stringCls->fieldOffsets.end()) valueIt = stringCls->fieldOffsets.find("value|[B");
                    if (valueIt != stringCls->fieldOffsets.end()) {
                        auto arrayCls = resolveClass("[C"); // Prefer [C (char[])
                        if (!arrayCls) arrayCls = resolveClass("[B"); // Fallback to [B
                    
                        if (arrayCls) {
                            auto arrayObj = HeapManager::getInstance().allocate(arrayCls);
                            std::vector<uint16_t> utf16 = utf8ToUtf16(strVal->bytes);
                            arrayObj->fields.resize(utf16.size());
                            for (size_t i = 0; i < utf16.size(); i++) {
                                arrayObj->fields[i] = utf16[i];
                            }
                            stringObj->fields[valueIt->second] = (int64_t)arrayObj;
                            
                            auto countIt = stringCls->fieldOffsets.find("count|I");
                            if (countIt != stringCls->fieldOffsets.end()) {
                                stringObj->fields[countIt->second] = utf16.size();
                            }
                        }
                    }
                    
                    auto offsetIt = stringCls->fieldOffsets.find("offset|I");
                    if (offsetIt != stringCls->fieldOffsets.end()) {
                        stringObj->fields[offsetIt->second] = 0;
                    }
                    
                    val.val.ref = stringObj;
                } else {
                    val.val.ref = nullptr;
                }
                
                frame->push(val);
            } else if (auto integer = std::dynamic_pointer_cast<ConstantInteger>(constant)) {
                JavaValue val; val.type = JavaValue::INT; val.val.i = integer->bytes;
                frame->push(val);
            } else if (auto flt = std::dynamic_pointer_cast<ConstantFloat>(constant)) {
                JavaValue val; val.type = JavaValue::FLOAT; val.val.f = flt->bytes;
                frame->push(val);
            } else if (auto clsConst = std::dynamic_pointer_cast<ConstantClass>(constant)) {
                auto nameInfo = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[clsConst->name_index]);
                resolveClass(nameInfo->bytes);
                
                auto classCls = resolveClass("java/lang/Class");
                JavaValue val;
                val.type = JavaValue::REFERENCE;
                if (classCls) {
                    val.val.ref = HeapManager::getInstance().allocate(classCls);
                } else {
                    val.val.ref = nullptr;
                }
                frame->push(val);
            } else {
                 LOG_ERROR("Unsupported LDC type at index " + std::to_string(index));
                 frame->push(JavaValue{JavaValue::INT, { .i = 0 }});
            }
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LDC_W] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            uint16_t index = codeReader.readU2();
            auto constant = frame->classFile->constant_pool[index];
            if (auto utf8 = std::dynamic_pointer_cast<ConstantString>(constant)) {
                auto strVal = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[utf8->string_index]);
                JavaValue val;
                val.type = JavaValue::REFERENCE; 
                val.strVal = strVal->bytes;
                
                auto stringCls = resolveClass("java/lang/String");
                if (stringCls) {
                    auto stringObj = HeapManager::getInstance().allocate(stringCls);
                    
                    auto valueIt = stringCls->fieldOffsets.find("value|[C");
                    if (valueIt != stringCls->fieldOffsets.end()) {
                        auto arrayCls = resolveClass("[C"); // Prefer [C
                        if (!arrayCls) arrayCls = resolveClass("[B"); // Fallback
                        
                        if (arrayCls) {
                            auto arrayObj = HeapManager::getInstance().allocate(arrayCls);
                            std::vector<uint16_t> utf16 = utf8ToUtf16(strVal->bytes);
                            arrayObj->fields.resize(utf16.size());
                            for (size_t i = 0; i < utf16.size(); i++) {
                                arrayObj->fields[i] = utf16[i];
                            }
                            stringObj->fields[valueIt->second] = (int64_t)arrayObj;
                            
                            auto countIt = stringCls->fieldOffsets.find("count|I");
                            if (countIt != stringCls->fieldOffsets.end()) {
                                stringObj->fields[countIt->second] = utf16.size();
                            }
                        }
                    }
                    
                    auto offsetIt = stringCls->fieldOffsets.find("offset|I");
                    if (offsetIt != stringCls->fieldOffsets.end()) {
                        stringObj->fields[offsetIt->second] = 0;
                    }
                    
                    val.val.ref = stringObj;
                } else {
                    val.val.ref = nullptr;
                }
                
                frame->push(val);
            } else if (auto integer = std::dynamic_pointer_cast<ConstantInteger>(constant)) {
                JavaValue val; val.type = JavaValue::INT; val.val.i = integer->bytes;
                frame->push(val);
            } else if (auto flt = std::dynamic_pointer_cast<ConstantFloat>(constant)) {
                JavaValue val; val.type = JavaValue::FLOAT; val.val.f = flt->bytes;
                frame->push(val);
            } else if (auto clsConst = std::dynamic_pointer_cast<ConstantClass>(constant)) {
                auto nameInfo = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[clsConst->name_index]);
                resolveClass(nameInfo->bytes);
                
                auto classCls = resolveClass("java/lang/Class");
                JavaValue val;
                val.type = JavaValue::REFERENCE;
                if (classCls) {
                    val.val.ref = HeapManager::getInstance().allocate(classCls);
                } else {
                    val.val.ref = nullptr;
                }
                frame->push(val);
            } else {
                 LOG_ERROR("Unsupported LDC_W type at index " + std::to_string(index));
                 frame->push(JavaValue{JavaValue::INT, { .i = 0 }});
            }
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_LDC2_W] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            uint16_t index = codeReader.readU2();
            auto constant = frame->classFile->constant_pool[index];
            if (auto lng = std::dynamic_pointer_cast<ConstantLong>(constant)) {
                JavaValue val; val.type = JavaValue::LONG; val.val.l = lng->bytes;
                frame->push(val);
            } else if (auto dbl = std::dynamic_pointer_cast<ConstantDouble>(constant)) {
                JavaValue val; val.type = JavaValue::DOUBLE; val.val.d = dbl->bytes;
                frame->push(val);
            } else {
                 LOG_ERROR("Unsupported LDC2_W type at index " + std::to_string(index));
            }
            break;
        } while(0);
        return true;
    };
}

}
}
