#include "Interpreter.hpp"
#include "Opcodes.hpp"
#include "ClassParser.hpp"
#include "HeapManager.hpp"
#include "NativeRegistry.hpp"
#include "Logger.hpp"
#include "EventLoop.hpp"
#include <iostream>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <functional>
#include <vector>

namespace j2me {
namespace core {

Interpreter::Interpreter(j2me::loader::JarLoader& loader) : jarLoader(loader) {}

std::shared_ptr<JavaClass> Interpreter::resolveClass(const std::string& className) {
    // std::cerr << "Resolving class: " << className << std::endl;
    // Check if already loaded
    auto it = loadedClasses.find(className);
    if (it != loadedClasses.end()) {
        // std::cerr << "Already loaded: " << className << std::endl;
        return it->second;
    }

    // Try to load from JAR
    // Class name in JAR usually ends with .class, but internal name doesn't
    std::string path = className + ".class";
    
    // Check if file exists
    if (!jarLoader.hasFile(path)) {
        // Check library loader if available
        if (libraryLoader && libraryLoader->hasFile(path)) {
             LOG_DEBUG("[Interpreter] Loading " + className + " from library loader");
             auto data = libraryLoader->getFile(path);
             if (data) {
                 try {
                     ClassParser parser;
                     auto rawFile = parser.parse(*data);
                     auto javaClass = std::make_shared<JavaClass>(rawFile);
                     
                     LOG_DEBUG("[Interpreter] Parsed " + className + " from library, fields=" + std::to_string(rawFile->fields.size()));
                     
                     if (rawFile->super_class != 0) {
                         auto superInfo = std::dynamic_pointer_cast<ConstantClass>(rawFile->constant_pool[rawFile->super_class]);
                         auto superNameInfo = std::dynamic_pointer_cast<ConstantUtf8>(rawFile->constant_pool[superInfo->name_index]);
                         std::string superName = superNameInfo->bytes;
             
                         if (superName != "java/lang/Object") {
                              auto superClass = resolveClass(superName);
                              javaClass->link(superClass);
                         } else {
                              javaClass->link(nullptr);
                         }
                     } else {
                         javaClass->link(nullptr);
                     }
             
                     loadedClasses[className] = javaClass;
                     return javaClass;
                 } catch (const std::exception& e) {
                     LOG_ERROR("Failed to parse library class " + className + ": " + e.what());
                 }
             }
        }

        // Mock java/lang/Object if missing (it's the root of everything)
        if (className == "java/lang/Object") {
             // Return a dummy ClassFile for Object so we stop recursion
             auto dummy = std::make_shared<ClassFile>();
             dummy->this_class = 0; // Invalid, but we won't read it
             
             // We need to construct a minimal valid JavaClass
             auto javaClass = std::make_shared<JavaClass>(dummy);
             javaClass->name = "java/lang/Object";
             javaClass->instanceSize = 0;
             loadedClasses[className] = javaClass;
             return javaClass;
        }

        // Mock java/lang/StringBuffer to ensure we use our Native implementation
        if (className == "java/lang/StringBuffer") {
             auto dummy = std::make_shared<ClassFile>();
             auto javaClass = std::make_shared<JavaClass>(dummy);
             javaClass->name = "java/lang/StringBuffer";
             javaClass->instanceSize = 1; // ID for native map
             
             // Helper to add method
             auto addMethod = [&](const std::string& name, const std::string& desc) {
                 MethodInfo m;
                 m.access_flags = 0x0101; // ACC_PUBLIC | ACC_NATIVE
                 
                 auto nameConst = std::make_shared<ConstantUtf8>(); nameConst->tag = CONSTANT_Utf8; nameConst->bytes = name;
                 dummy->constant_pool.push_back(nameConst);
                 m.name_index = dummy->constant_pool.size() - 1;
                 
                 auto descConst = std::make_shared<ConstantUtf8>(); descConst->tag = CONSTANT_Utf8; descConst->bytes = desc;
                 dummy->constant_pool.push_back(descConst);
                 m.descriptor_index = dummy->constant_pool.size() - 1;
                 
                 dummy->methods.push_back(m);
             };

             addMethod("<init>", "()V");
             addMethod("append", "(Ljava/lang/String;)Ljava/lang/StringBuffer;");
             addMethod("append", "(I)Ljava/lang/StringBuffer;");
             addMethod("append", "(Ljava/lang/Object;)Ljava/lang/StringBuffer;");
             addMethod("toString", "()Ljava/lang/String;");

             loadedClasses[className] = javaClass;
             return javaClass;
        }

        // Mock java/lang/StringBuilder to ensure we use our Native implementation
        if (className == "java/lang/StringBuilder") {
             auto dummy = std::make_shared<ClassFile>();
             auto javaClass = std::make_shared<JavaClass>(dummy);
             javaClass->name = "java/lang/StringBuilder";
             javaClass->instanceSize = 0; // We use fields vector dynamically
             
             // Helper to add method
             auto addMethod = [&](const std::string& name, const std::string& desc) {
                 MethodInfo m;
                 m.access_flags = 0x0101; // ACC_PUBLIC | ACC_NATIVE
                 
                 auto nameConst = std::make_shared<ConstantUtf8>(); nameConst->tag = CONSTANT_Utf8; nameConst->bytes = name;
                 dummy->constant_pool.push_back(nameConst);
                 m.name_index = dummy->constant_pool.size() - 1;
                 
                 auto descConst = std::make_shared<ConstantUtf8>(); descConst->tag = CONSTANT_Utf8; descConst->bytes = desc;
                 dummy->constant_pool.push_back(descConst);
                 m.descriptor_index = dummy->constant_pool.size() - 1;
                 
                 dummy->methods.push_back(m);
             };

             addMethod("<init>", "()V");
             addMethod("append", "(Ljava/lang/String;)Ljava/lang/StringBuilder;");
             addMethod("append", "(I)Ljava/lang/StringBuilder;");
             addMethod("append", "(Ljava/lang/Object;)Ljava/lang/StringBuilder;");
             addMethod("toString", "()Ljava/lang/String;");

             loadedClasses[className] = javaClass;
             return javaClass;
        }

        // Mock java/io/InputStream for resource loading
        if (className == "java/io/InputStream") {
             auto dummy = std::make_shared<ClassFile>();
             auto javaClass = std::make_shared<JavaClass>(dummy);
             javaClass->name = "java/io/InputStream";
             javaClass->instanceSize = 1; // One field for native stream ID
             
             // Helper to add method
             auto addMethod = [&](const std::string& name, const std::string& desc) {
                 MethodInfo m;
                 m.access_flags = 0x0101; // ACC_PUBLIC | ACC_NATIVE
                 
                 auto nameConst = std::make_shared<ConstantUtf8>(); nameConst->tag = CONSTANT_Utf8; nameConst->bytes = name;
                 dummy->constant_pool.push_back(nameConst);
                 m.name_index = dummy->constant_pool.size() - 1;
                 
                 auto descConst = std::make_shared<ConstantUtf8>(); descConst->tag = CONSTANT_Utf8; descConst->bytes = desc;
                 dummy->constant_pool.push_back(descConst);
                 m.descriptor_index = dummy->constant_pool.size() - 1;
                 
                 dummy->methods.push_back(m);
             };

             addMethod("read", "()I");
             addMethod("read", "([B)I");
             addMethod("close", "()V");

             loadedClasses[className] = javaClass;
             return javaClass;
        }

        // Mock java/lang/String for string operations
        if (className == "java/lang/String") {
             auto dummy = std::make_shared<ClassFile>();
             auto javaClass = std::make_shared<JavaClass>(dummy);
             javaClass->name = "java/lang/String";
             javaClass->instanceSize = 3; // Three fields: value, offset, count
             
             // Helper to add field
             auto addField = [&](const std::string& name, const std::string& desc, uint16_t access_flags) {
                 FieldInfo f;
                 f.access_flags = access_flags;
                 
                 auto nameConst = std::make_shared<ConstantUtf8>(); nameConst->tag = CONSTANT_Utf8; nameConst->bytes = name;
                 dummy->constant_pool.push_back(nameConst);
                 f.name_index = dummy->constant_pool.size() - 1;
                 
                 auto descConst = std::make_shared<ConstantUtf8>(); descConst->tag = CONSTANT_Utf8; descConst->bytes = desc;
                 dummy->constant_pool.push_back(descConst);
                 f.descriptor_index = dummy->constant_pool.size() - 1;
                 
                 dummy->fields.push_back(f);
                 LOG_DEBUG("[Mock String] Added field: " + name + " desc=" + desc + " total fields=" + std::to_string(dummy->fields.size()));
             };
             
             // Add fields
             addField("value", "[B", 0x0002); // private byte[] value
             addField("offset", "I", 0x0002); // private int offset
             addField("count", "I", 0x0002); // private int count
             
             // Manually set fieldOffsets since we're not calling link()
             javaClass->fieldOffsets["value"] = 0;
             javaClass->fieldOffsets["offset"] = 1;
             javaClass->fieldOffsets["count"] = 2;
             
             LOG_DEBUG("[Mock String] Created mock String class with fieldOffsets size=" + std::to_string(javaClass->fieldOffsets.size()) + " rawFile->fields.size()=" + std::to_string(dummy->fields.size()));
             
             // Helper to add method
             auto addMethod = [&](const std::string& name, const std::string& desc) {
                 MethodInfo m;
                 m.access_flags = 0x0101; // ACC_PUBLIC | ACC_NATIVE
                 
                 auto nameConst = std::make_shared<ConstantUtf8>(); nameConst->tag = CONSTANT_Utf8; nameConst->bytes = name;
                 dummy->constant_pool.push_back(nameConst);
                 m.name_index = dummy->constant_pool.size() - 1;
                 
                 auto descConst = std::make_shared<ConstantUtf8>(); descConst->tag = CONSTANT_Utf8; descConst->bytes = desc;
                 dummy->constant_pool.push_back(descConst);
                 m.descriptor_index = dummy->constant_pool.size() - 1;
                 
                 dummy->methods.push_back(m);
             };

             addMethod("<init>", "()V");
             addMethod("getBytes", "()[B");

             loadedClasses[className] = javaClass;
             return javaClass;
        }

        // Generic array class handling
        if (className.length() > 0 && className[0] == '[') {
             auto dummy = std::make_shared<ClassFile>();
             auto javaClass = std::make_shared<JavaClass>(dummy);
             javaClass->name = className;
             javaClass->instanceSize = 0; // Arrays use dynamic field storage
             
             loadedClasses[className] = javaClass;
             return javaClass;
        }
        
        LOG_ERROR("Class not found: " + className);
        throw std::runtime_error("Class not found: " + className);
    }

    auto data = jarLoader.getFile(path);
    if (!data) {
        LOG_ERROR("Class not found: " + className);
        throw std::runtime_error("Class not found: " + className);
    }

    try {
        ClassParser parser;
        auto rawFile = parser.parse(*data);
        auto javaClass = std::make_shared<JavaClass>(rawFile);
        
        // Link with superclass (recursive load)
        if (rawFile->super_class != 0) {
            auto superInfo = std::dynamic_pointer_cast<ConstantClass>(rawFile->constant_pool[rawFile->super_class]);
            auto superNameInfo = std::dynamic_pointer_cast<ConstantUtf8>(rawFile->constant_pool[superInfo->name_index]);
            std::string superName = superNameInfo->bytes;

            if (superName != "java/lang/Object") {
                 auto superClass = resolveClass(superName);
                 javaClass->link(superClass);
            } else {
                 // Even if it is Object, we need to ensure Object is loaded?
                 // But link(nullptr) is for Object itself or direct subclasses of Object (if we treat Object as root with no fields).
                 // Actually, if we load Object.class from JAR, we should resolve it too to get its size (0).
                 // But my previous logic was:
                 // if superName != "java/lang/Object" -> resolve.
                 // else -> link(nullptr).
                 // This means direct subclasses of Object don't link to Object class structure.
                 // They just start with offset 0.
                 // This is fine as long as Object has no fields.
                 // But we should probably verify Object exists.
                 javaClass->link(nullptr);
            }
        } else {
            // This is java/lang/Object (or invalid)
            javaClass->link(nullptr);
        }

        loadedClasses[className] = javaClass;
        
        // Load all referenced classes from constant pool (for dependencies)
        // LOG_DEBUG("[Interpreter] Loading dependencies for: " + className);
        // int checkedCount = 0;
        // for (const auto& cpEntry : rawFile->constant_pool) {
        //     if (cpEntry && cpEntry->tag == CONSTANT_Class) {
        //         auto classInfo = std::dynamic_pointer_cast<ConstantClass>(cpEntry);
        //         if (classInfo) {
        //             if (classInfo->name_index > 0 && classInfo->name_index < rawFile->constant_pool.size()) {
        //                 auto nameInfo = std::dynamic_pointer_cast<ConstantUtf8>(rawFile->constant_pool[classInfo->name_index]);
        //                 if (nameInfo) {
        //                     std::string refClassName = nameInfo->bytes;
        //                     checkedCount++;
        //                     // Skip self and java/lang/Object (already handled)
        //                     // Also skip array types (start with '[')
        //                     // Also skip invalid class names (descriptors, short names, etc.)
        //                     if (refClassName != className && refClassName != "java/lang/Object" && !refClassName.empty() && refClassName[0] != '[') {
        //                         // Validate class name before attempting to load
        //                         bool valid = isValidClassName(refClassName);
        //                         // LOG_DEBUG("[Interpreter]   Found class name: " + refClassName + " (valid: " + (valid ? "yes" : "no") + ")");
        //                         if (!valid) {
        //                             // LOG_DEBUG("[Interpreter]   Skipping invalid class name: " + refClassName);
        //                             continue;
        //                         }
        //                         // LOG_DEBUG("[Interpreter]   Checking dependency: " + refClassName);
        //                         // Only load if not already loaded
        //                         if (loadedClasses.find(refClassName) == loadedClasses.end()) {
        //                             try {
        //                                 resolveClass(refClassName);
        //                             } catch (const std::exception& e) {
        //                                 LOG_ERROR("[Interpreter] Warning: Failed to load dependency " + refClassName + ": " + e.what());
        //                             }
        //                         }
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }
        // LOG_DEBUG("[Interpreter] Checked " + std::to_string(checkedCount) + " CONSTANT_Class entries");
        
        // Initialize the class (execute <clinit>)
        // Initialization should be done on demand (when class is actually used)
        // But for main class, we might need it. 
        // Current implementation calls initializeClass inside opcodes.
        // However, resolveClass is called by opcodes.
        // So if we remove initializeClass here, we might miss initialization if opcode assumes resolveClass initializes.
        // Opcodes: OP_NEW -> resolveClass -> initializeClass?
        // Check OP_NEW.
        // OP_NEW calls resolveClass. Does NOT call initializeClass explicitly in my previous read?
        // Let's check OP_NEW again.
        
        return javaClass;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse class " + className + ": " + e.what());
        throw;
    }
}

std::optional<JavaValue> Interpreter::execute(std::shared_ptr<StackFrame> frame) {
    if (!frame) return std::nullopt;
    
    // Get method name for logging
    std::string methodName = "<unknown>";
    if (frame->method.name_index > 0 && frame->method.name_index <= frame->classFile->constant_pool.size()) {
        auto nameInfo = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[frame->method.name_index]);
        if (nameInfo) {
            methodName = nameInfo->bytes;
        }
    }
    LOG_DEBUG("[Interpreter] Entering method: " + methodName);
    
    // Code attribute
    // We need to parse it from attributes if not cached
    std::vector<uint8_t> code;
    for (const auto& attr : frame->method.attributes) {
        auto nameInfo = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[attr.attribute_name_index]);
        if (nameInfo && nameInfo->bytes == "Code") {
             util::DataReader attrReader(attr.info);
             attrReader.readU2(); // max_stack
             attrReader.readU2(); // max_locals
             uint32_t codeLength = attrReader.readU4();
             code = attrReader.readBytes(codeLength);
             break;
        }
    }
    
    if (code.empty()) {
        // Native?
        LOG_DEBUG("[Interpreter] No code attribute (native?) for method: " + methodName);
        return std::nullopt;
    }

    util::DataReader codeReader(code);
    
    LOG_DEBUG("[Interpreter] Starting execution, code size: " + std::to_string(code.size()) + " bytes");

    while (codeReader.tell() < code.size()) {
        // Check for global exit signal
        if (EventLoop::getInstance().shouldExit()) {
            LOG_INFO("[Interpreter] VM Exit requested. Stopping execution.");
            return std::nullopt; // Or throw an exception to unwind stack faster
        }

        try {
            std::optional<JavaValue> ret;
            bool shouldContinue = executeInstruction(frame, codeReader, ret);
            // LOG_DEBUG("[Interpreter] executeInstruction returned: " + std::string(shouldContinue ? "true" : "false") + " at PC " + std::to_string(codeReader.tell()));
            if (!shouldContinue) {
                LOG_DEBUG("[Interpreter] Stopping execution (return instruction or error)");
                if (ret) {
                    LOG_DEBUG("[Interpreter] Returning value from method");
                    return ret;
                }
                return std::nullopt;
            }
        } catch (const std::exception& e) {
            std::string className = "<unknown>";
            if (frame->classFile->this_class > 0) {
                 auto classInfo = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[frame->classFile->this_class]);
                 if (classInfo) {
                     auto nameInfo = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classInfo->name_index]);
                     if (nameInfo) className = nameInfo->bytes;
                 }
            }
            LOG_ERROR("Runtime Exception in method " + className + "." + methodName + " at PC " + std::to_string(codeReader.tell()) + ": " + std::string(e.what()));
            throw;
        }
    }
    
    LOG_DEBUG("[Interpreter] Execution completed normally");
    return std::nullopt;
}

void Interpreter::initializeClass(std::shared_ptr<JavaClass> cls) {
    if (cls->initialized) {
        return;
    }
    
    // Prevent circular initialization
    if (cls->initializing) {
        LOG_DEBUG("[Interpreter] Class already being initialized: " + cls->name);
        return;
    }
    
    cls->initializing = true;
    LOG_DEBUG("[Interpreter] Initializing class: " + cls->name);
    
    // First, initialize superclass if exists
    if (cls->superClass) {
        initializeClass(cls->superClass);
    }
    
    // Find and execute <clinit> method
    for (const auto& method : cls->rawFile->methods) {
        auto name = std::dynamic_pointer_cast<ConstantUtf8>(cls->rawFile->constant_pool[method.name_index]);
        if (name && name->bytes == "<clinit>") {
            LOG_DEBUG("[Interpreter] Executing <clinit> for: " + cls->name);
            try {
                auto frame = std::make_shared<StackFrame>(method, cls->rawFile);
                execute(frame);
            } catch (const std::exception& e) {
                LOG_ERROR("[Interpreter] Error executing <clinit> for " + cls->name + ": " + e.what());
                throw;
            }
            break;
        }
    }
    
    cls->initialized = true;
    cls->initializing = false;
    LOG_DEBUG("[Interpreter] Class initialized: " + cls->name);
}

bool Interpreter::executeInstruction(std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, std::optional<JavaValue>& returnVal) {
    static uint64_t instructionCount = 0;
    instructionCount++;
    if (instructionCount % 100000 == 0) {
        std::string methodName = "<unknown>";
        if (frame->method.name_index > 0 && frame->method.name_index <= frame->classFile->constant_pool.size()) {
            auto nameInfo = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[frame->method.name_index]);
            if (nameInfo) methodName = nameInfo->bytes;
        }
        
        std::string className = "<unknown>";
        if (frame->classFile->this_class > 0 && frame->classFile->this_class < frame->classFile->constant_pool.size()) {
             auto clsInfo = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[frame->classFile->this_class]);
             if (clsInfo) {
                 auto nameInfo = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[clsInfo->name_index]);
                 if (nameInfo) className = nameInfo->bytes;
             }
        }

        std::cout << "[Heartbeat] Ops: " << instructionCount << " | Executing: " << className << "." << methodName << " | Stack: " << frame->stackSize() << std::endl;
    }

    uint8_t opcode = codeReader.readU1();
    
    // Debug pal/b execution
    if (frame->classFile->this_class > 0) {
        auto clsInfo = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[frame->classFile->this_class]);
        if (clsInfo) {
            auto nameInfo = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[clsInfo->name_index]);
            if (nameInfo && nameInfo->bytes == "pal/b") {
                // std::cout << "[Trace] pal/b PC: " << (codeReader.tell() - 1) << " Op: " << std::hex << (int)opcode << std::dec << std::endl;
            }
        }
    }

    // LOG_DEBUG("[Interpreter] Opcode: " + std::to_string((int)opcode) + " at PC " + std::to_string(codeReader.tell()-1) + " Stack: " + std::to_string(frame->stackSize()));
    
    switch (opcode) {
        case OP_NOP: break;
        
        // --- Constants ---
        case OP_ACONST_NULL: {
            JavaValue v; v.type = JavaValue::REFERENCE; v.val.ref = nullptr;
            frame->push(v);
            break;
        }
        case OP_ICONST_M1: { JavaValue v; v.type = JavaValue::INT; v.val.i = -1; frame->push(v); break; }
        case OP_ICONST_0:  { JavaValue v; v.type = JavaValue::INT; v.val.i = 0; frame->push(v); break; }
        case OP_ICONST_1:  { JavaValue v; v.type = JavaValue::INT; v.val.i = 1; frame->push(v); break; }
        case OP_ICONST_2:  { JavaValue v; v.type = JavaValue::INT; v.val.i = 2; frame->push(v); break; }
        case OP_ICONST_3:  { JavaValue v; v.type = JavaValue::INT; v.val.i = 3; frame->push(v); break; }
        case OP_ICONST_4:  { JavaValue v; v.type = JavaValue::INT; v.val.i = 4; frame->push(v); break; }
        case OP_ICONST_5:  { JavaValue v; v.type = JavaValue::INT; v.val.i = 5; frame->push(v); break; }
        case OP_LCONST_0:  { JavaValue v; v.type = JavaValue::LONG; v.val.l = 0; frame->push(v); break; }
        case OP_LCONST_1:  { JavaValue v; v.type = JavaValue::LONG; v.val.l = 1; frame->push(v); break; }
        case OP_FCONST_0:  { JavaValue v; v.type = JavaValue::FLOAT; v.val.f = 0.0f; frame->push(v); break; }
        case OP_FCONST_1:  { JavaValue v; v.type = JavaValue::FLOAT; v.val.f = 1.0f; frame->push(v); break; }
        case OP_FCONST_2:  { JavaValue v; v.type = JavaValue::FLOAT; v.val.f = 2.0f; frame->push(v); break; }
        case OP_DCONST_0:  { JavaValue v; v.type = JavaValue::DOUBLE; v.val.d = 0.0; frame->push(v); break; }
        case OP_DCONST_1:  { JavaValue v; v.type = JavaValue::DOUBLE; v.val.d = 1.0; frame->push(v); break; }
        
        case OP_BIPUSH: {
            int8_t byteVal = (int8_t)codeReader.readU1();
            JavaValue v; v.type = JavaValue::INT; v.val.i = byteVal;
            frame->push(v);
            break;
        }
        case OP_SIPUSH: {
            int16_t shortVal = (int16_t)codeReader.readU2();
            JavaValue v; v.type = JavaValue::INT; v.val.i = shortVal;
            frame->push(v);
            break;
        }
        
        case OP_LDC: {
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
                        arrayObj->fields.resize(strVal->bytes.length());
                        for (size_t i = 0; i < strVal->bytes.length(); i++) {
                            arrayObj->fields[i] = (uint16_t)(uint8_t)strVal->bytes[i];
                        }
                        stringObj->fields[valueIt->second] = (int64_t)arrayObj;
                    }
                }
                    
                    auto offsetIt = stringCls->fieldOffsets.find("offset|I");
                    if (offsetIt != stringCls->fieldOffsets.end()) {
                        stringObj->fields[offsetIt->second] = 0;
                    }
                    
                    auto countIt = stringCls->fieldOffsets.find("count|I");
                    if (countIt != stringCls->fieldOffsets.end()) {
                        stringObj->fields[countIt->second] = strVal->bytes.length();
                    }
                    
                    val.val.ref = stringObj;
                } else {
                    val.val.ref = nullptr;
                }
                
                frame->push(val);
            } else if (auto integer = std::dynamic_pointer_cast<ConstantInteger>(constant)) {
                JavaValue val; val.type = JavaValue::INT; val.val.i = integer->bytes;
                // std::cerr << "[LDC] Loading int constant #" << index << ": " << integer->bytes << ", type=" << val.type << std::endl;
                frame->push(val);
            } else if (auto flt = std::dynamic_pointer_cast<ConstantFloat>(constant)) {
                JavaValue val; val.type = JavaValue::FLOAT; val.val.f = flt->bytes;
                // std::cerr << "[LDC] Loading float constant #" << index << ": " << flt->bytes << ", type=" << val.type << std::endl;
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
                 std::cerr << "Unsupported LDC type at index " << (int)index << std::endl;
                 // Push dummy to avoid stack underflow
                 frame->push(JavaValue{JavaValue::INT, { .i = 0 }});
            }
            break;
        }

        case OP_LDC_W: {
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
                            arrayObj->fields.resize(strVal->bytes.length());
                            for (size_t i = 0; i < strVal->bytes.length(); i++) {
                                arrayObj->fields[i] = (uint16_t)(uint8_t)strVal->bytes[i];
                            }
                            stringObj->fields[valueIt->second] = (int64_t)arrayObj;
                        }
                    }
                    
                    auto offsetIt = stringCls->fieldOffsets.find("offset|I");
                    if (offsetIt != stringCls->fieldOffsets.end()) {
                        stringObj->fields[offsetIt->second] = 0;
                    }
                    
                    auto countIt = stringCls->fieldOffsets.find("count|I");
                    if (countIt != stringCls->fieldOffsets.end()) {
                        stringObj->fields[countIt->second] = strVal->bytes.length();
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
                 std::cerr << "Unsupported LDC_W type at index " << (int)index << std::endl;
                 frame->push(JavaValue{JavaValue::INT, { .i = 0 }});
            }
            break;
        }

        case OP_LDC2_W: {
            uint16_t index = codeReader.readU2();
            auto constant = frame->classFile->constant_pool[index];
            if (auto lng = std::dynamic_pointer_cast<ConstantLong>(constant)) {
                JavaValue val; val.type = JavaValue::LONG; val.val.l = lng->bytes;
                frame->push(val);
            } else if (auto dbl = std::dynamic_pointer_cast<ConstantDouble>(constant)) {
                JavaValue val; val.type = JavaValue::DOUBLE; val.val.d = dbl->bytes;
                frame->push(val);
            } else {
                 std::cerr << "Unsupported LDC2_W type at index " << (int)index << std::endl;
            }
            break;
        }

        // --- Loads ---
        case OP_ILOAD: { uint8_t idx = codeReader.readU1(); frame->push(frame->getLocal(idx)); break; }
        case OP_ILOAD_0: frame->push(frame->getLocal(0)); break;
        case OP_ILOAD_1: frame->push(frame->getLocal(1)); break;
        case OP_ILOAD_2: frame->push(frame->getLocal(2)); break;
        case OP_ILOAD_3: frame->push(frame->getLocal(3)); break;

        case OP_LLOAD: { uint8_t idx = codeReader.readU1(); frame->push(frame->getLocal(idx)); break; }
        case OP_LLOAD_0: {
             frame->push(frame->getLocal(0));
             break;
        }
        case OP_LLOAD_1: {
             frame->push(frame->getLocal(1));
             break;
        }
        case OP_LLOAD_2: {
             frame->push(frame->getLocal(2));
             break;
        }
        case OP_LLOAD_3: {
             frame->push(frame->getLocal(3));
             break;
        }
        
        case OP_FLOAD: { uint8_t idx = codeReader.readU1(); frame->push(frame->getLocal(idx)); break; }
        case OP_FLOAD_0: {
             frame->push(frame->getLocal(0));
             break;
        }
        case OP_FLOAD_1: {
             frame->push(frame->getLocal(1));
             break;
        }
        case OP_FLOAD_2: {
             frame->push(frame->getLocal(2));
             break;
        }
        case OP_FLOAD_3: {
             frame->push(frame->getLocal(3));
             break;
        }
        
        case OP_DLOAD: { uint8_t idx = codeReader.readU1(); frame->push(frame->getLocal(idx)); break; }
        case OP_DLOAD_0: {
             frame->push(frame->getLocal(0));
             break;
        }
        case OP_DLOAD_1: {
             frame->push(frame->getLocal(1));
             break;
        }
        case OP_DLOAD_2: {
             frame->push(frame->getLocal(2));
             break;
        }
        case OP_DLOAD_3: {
             frame->push(frame->getLocal(3));
             break;
        }
        
        case OP_ALOAD: { uint8_t idx = codeReader.readU1(); frame->push(frame->getLocal(idx)); break; }
        case OP_ALOAD_0: {
            JavaValue v = frame->getLocal(0);
            // std::cout << "[OP_ALOAD_0] Loading local[0]: type=" << v.type << " ref=" << v.val.ref << std::endl;
            frame->push(v); 
            break;
        }
        case OP_ALOAD_1: frame->push(frame->getLocal(1)); break;
        case OP_ALOAD_2: frame->push(frame->getLocal(2)); break;
        case OP_ALOAD_3: frame->push(frame->getLocal(3)); break;



        // --- Stores ---
        case OP_ISTORE: { uint8_t idx = codeReader.readU1(); frame->setLocal(idx, frame->pop()); break; }
        case OP_ISTORE_0: frame->setLocal(0, frame->pop()); break;
        case OP_ISTORE_1: frame->setLocal(1, frame->pop()); break;
        case OP_ISTORE_2: frame->setLocal(2, frame->pop()); break;
        case OP_ISTORE_3: frame->setLocal(3, frame->pop()); break;

        case OP_ASTORE: { uint8_t idx = codeReader.readU1(); frame->setLocal(idx, frame->pop()); break; }
        case OP_ASTORE_0: frame->setLocal(0, frame->pop()); break;
        case OP_ASTORE_1: frame->setLocal(1, frame->pop()); break;
        case OP_ASTORE_2: frame->setLocal(2, frame->pop()); break;
        case OP_ASTORE_3: frame->setLocal(3, frame->pop()); break;
        
        // --- Wide Stores (for long, float, double) ---
        // For now, we'll implement LSTORE, FSTORE, DSTORE using the same pattern
        case OP_LSTORE: {
            uint8_t idx = codeReader.readU1();
            frame->setLocal(idx, frame->pop());
            break;
        }
        case OP_LSTORE_0: {
            frame->setLocal(0, frame->pop());
            break;
        }
        case OP_LSTORE_1: {
            frame->setLocal(1, frame->pop());
            break;
        }
        case OP_LSTORE_2: {
            frame->setLocal(2, frame->pop());
            break;
        }
        case OP_LSTORE_3: {
            frame->setLocal(3, frame->pop());
            break;
        }
        case OP_FSTORE: {
            uint8_t idx = codeReader.readU1();
            frame->setLocal(idx, frame->pop());
            break;
        }
        case OP_FSTORE_0: {
            frame->setLocal(0, frame->pop());
            break;
        }
        case OP_FSTORE_1: {
            frame->setLocal(1, frame->pop());
            break;
        }
        case OP_FSTORE_2: {
            frame->setLocal(2, frame->pop());
            break;
        }
        case OP_FSTORE_3: {
            frame->setLocal(3, frame->pop());
            break;
        }
        case OP_DSTORE: {
            uint8_t idx = codeReader.readU1();
            frame->setLocal(idx, frame->pop());
            break;
        }
        case OP_DSTORE_0: {
            frame->setLocal(0, frame->pop());
            break;
        }
        case OP_DSTORE_1: {
            frame->setLocal(1, frame->pop());
            break;
        }
        case OP_DSTORE_2: {
            frame->setLocal(2, frame->pop());
            break;
        }
        case OP_DSTORE_3: {
            frame->setLocal(3, frame->pop());
            break;
        }
        
        // --- Stack ---
        case OP_POP: frame->pop(); break;
        case OP_POP2: {
            JavaValue v = frame->pop();
            if (v.type != JavaValue::LONG && v.type != JavaValue::DOUBLE) {
                frame->pop();
            }
            break;
        }
        case OP_DUP: frame->push(frame->peek()); break;
        case OP_DUP_X1: {
            JavaValue v1 = frame->pop();
            JavaValue v2 = frame->pop();
            frame->push(v1);
            frame->push(v2);
            frame->push(v1);
            break;
        }
        case OP_DUP_X2: {
            JavaValue v1 = frame->pop();
            JavaValue v2 = frame->pop();
            if (v2.type == JavaValue::LONG || v2.type == JavaValue::DOUBLE) {
                // Form 2: v1, v2(long) -> v1, v2, v1
                frame->push(v1);
                frame->push(v2);
                frame->push(v1);
            } else {
                // Form 1: v1, v2, v3 -> v1, v2, v3, v1
                JavaValue v3 = frame->pop();
                frame->push(v1);
                frame->push(v3);
                frame->push(v2);
                frame->push(v1);
            }
            break;
        }
        case OP_DUP2: {
            JavaValue v1 = frame->pop();
            if (v1.type == JavaValue::LONG || v1.type == JavaValue::DOUBLE) {
                frame->push(v1);
                frame->push(v1);
            } else {
                JavaValue v2 = frame->pop();
                frame->push(v2);
                frame->push(v1);
                frame->push(v2);
                frame->push(v1);
            }
            break;
        }
        case OP_DUP2_X1: {
            JavaValue v1 = frame->pop();
            if (v1.type == JavaValue::LONG || v1.type == JavaValue::DOUBLE) {
                // Form 2: v1(long), v2 -> v1, v2, v1
                JavaValue v2 = frame->pop();
                frame->push(v1);
                frame->push(v2);
                frame->push(v1);
            } else {
                // Form 1: v1, v2, v3 -> v2, v1, v3, v2, v1
                JavaValue v2 = frame->pop();
                JavaValue v3 = frame->pop();
                frame->push(v2);
                frame->push(v1);
                frame->push(v3);
                frame->push(v2);
                frame->push(v1);
            }
            break;
        }
        case OP_DUP2_X2: {
            JavaValue v1 = frame->pop();
            if (v1.type == JavaValue::LONG || v1.type == JavaValue::DOUBLE) {
                JavaValue v2 = frame->pop();
                if (v2.type == JavaValue::LONG || v2.type == JavaValue::DOUBLE) {
                     // Form 4: v1(long), v2(long) -> v1, v2, v1
                     frame->push(v1);
                     frame->push(v2);
                     frame->push(v1);
                } else {
                     // Form 2: v1(long), v2, v3 -> v1, v2, v3, v1
                     JavaValue v3 = frame->pop();
                     frame->push(v1);
                     frame->push(v3);
                     frame->push(v2);
                     frame->push(v1);
                }
            } else {
                JavaValue v2 = frame->pop();
                JavaValue v3 = frame->pop();
                if (v3.type == JavaValue::LONG || v3.type == JavaValue::DOUBLE) {
                    // Form 3: v1, v2, v3(long) -> v2, v1, v3, v2, v1
                    frame->push(v2);
                    frame->push(v1);
                    frame->push(v3);
                    frame->push(v2);
                    frame->push(v1);
                } else {
                    // Form 1: v1, v2, v3, v4 -> v2, v1, v4, v3, v2, v1
                    JavaValue v4 = frame->pop();
                    frame->push(v2);
                    frame->push(v1);
                    frame->push(v4);
                    frame->push(v3);
                    frame->push(v2);
                    frame->push(v1);
                }
            }
            break;
        }
        case OP_SWAP: {
            JavaValue v1 = frame->pop();
            JavaValue v2 = frame->pop();
            frame->push(v1);
            frame->push(v2);
            break;
        }

        // --- Math ---
        case OP_IADD: {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 + v2;
            frame->push(res);
            break;
        }
        case OP_LADD: {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 + v2;
            frame->push(res);
            break;
        }
        case OP_FADD: {
            float v2 = frame->pop().val.f;
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = v1 + v2;
            frame->push(res);
            break;
        }
        case OP_DADD: {
            double v2 = frame->pop().val.d;
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = v1 + v2;
            frame->push(res);
            break;
        }

        case OP_ISUB: {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 - v2;
            frame->push(res);
            break;
        }
        case OP_LSUB: {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 - v2;
            frame->push(res);
            break;
        }
        case OP_FSUB: {
            float v2 = frame->pop().val.f;
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = v1 - v2;
            frame->push(res);
            break;
        }
        case OP_DSUB: {
            double v2 = frame->pop().val.d;
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = v1 - v2;
            frame->push(res);
            break;
        }

        case OP_IMUL: {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 * v2;
            frame->push(res);
            break;
        }
        case OP_LMUL: {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 * v2;
            frame->push(res);
            break;
        }
        case OP_FMUL: {
            float v2 = frame->pop().val.f;
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = v1 * v2;
            frame->push(res);
            break;
        }
        case OP_DMUL: {
            double v2 = frame->pop().val.d;
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = v1 * v2;
            frame->push(res);
            break;
        }

        case OP_IDIV: {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            if (v2 == 0) throw std::runtime_error("ArithmeticException: / by zero");
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 / v2;
            frame->push(res);
            break;
        }
        case OP_LDIV: {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            if (v2 == 0) throw std::runtime_error("ArithmeticException: / by zero");
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 / v2;
            frame->push(res);
            break;
        }
        case OP_FDIV: {
            float v2 = frame->pop().val.f;
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = v1 / v2;
            frame->push(res);
            break;
        }
        case OP_DDIV: {
            double v2 = frame->pop().val.d;
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = v1 / v2;
            frame->push(res);
            break;
        }

        case OP_IREM: {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            if (v2 == 0) throw std::runtime_error("ArithmeticException: / by zero");
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 % v2;
            frame->push(res);
            break;
        }
        case OP_LREM: {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            if (v2 == 0) throw std::runtime_error("ArithmeticException: / by zero");
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 % v2;
            frame->push(res);
            break;
        }
        case OP_FREM: {
            float v2 = frame->pop().val.f;
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = std::fmod(v1, v2);
            frame->push(res);
            break;
        }
        case OP_DREM: {
            double v2 = frame->pop().val.d;
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = std::fmod(v1, v2);
            frame->push(res);
            break;
        }

        case OP_INEG: {
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = -v1;
            frame->push(res);
            break;
        }
        case OP_LNEG: {
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = -v1;
            frame->push(res);
            break;
        }
        case OP_FNEG: {
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = -v1;
            frame->push(res);
            break;
        }
        case OP_DNEG: {
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = -v1;
            frame->push(res);
            break;
        }

        case OP_ISHL: {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 << (v2 & 0x1F);
            frame->push(res);
            break;
        }
        case OP_LSHL: {
            int32_t v2 = frame->pop().val.i;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 << (v2 & 0x3F);
            frame->push(res);
            break;
        }

        case OP_ISHR: {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 >> (v2 & 0x1F);
            frame->push(res);
            break;
        }
        case OP_LSHR: {
            int32_t v2 = frame->pop().val.i;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 >> (v2 & 0x3F);
            frame->push(res);
            break;
        }

        case OP_IUSHR: {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = (uint32_t)v1 >> (v2 & 0x1F);
            frame->push(res);
            break;
        }
        case OP_LUSHR: {
            int32_t v2 = frame->pop().val.i;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = (uint64_t)v1 >> (v2 & 0x3F);
            frame->push(res);
            break;
        }

        case OP_IAND: {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 & v2;
            frame->push(res);
            break;
        }
        case OP_LAND: {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 & v2;
            frame->push(res);
            break;
        }

        case OP_IOR: {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 | v2;
            frame->push(res);
            break;
        }
        case OP_LOR: {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 | v2;
            frame->push(res);
            break;
        }

        case OP_IXOR: {
            int32_t v2 = frame->pop().val.i;
            int32_t v1 = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = v1 ^ v2;
            frame->push(res);
            break;
        }
        case OP_LXOR: {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = v1 ^ v2;
            frame->push(res);
            break;
        }

        // --- Conversions ---
        case OP_I2L: {
            int32_t v = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = (int64_t)v;
            frame->push(res);
            break;
        }
        case OP_I2F: {
            int32_t v = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = (float)v;
            frame->push(res);
            break;
        }
        case OP_I2D: {
            int32_t v = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = (double)v;
            frame->push(res);
            break;
        }
        case OP_L2I: {
            int64_t v = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::INT; res.val.i = (int32_t)v;
            frame->push(res);
            break;
        }
        case OP_L2F: {
            int64_t v = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = (float)v;
            frame->push(res);
            break;
        }
        case OP_L2D: {
            int64_t v = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = (double)v;
            frame->push(res);
            break;
        }
        case OP_F2I: {
            float v = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::INT; res.val.i = (int32_t)v;
            frame->push(res);
            break;
        }
        case OP_F2L: {
            float v = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = (int64_t)v;
            frame->push(res);
            break;
        }
        case OP_F2D: {
            float v = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::DOUBLE; res.val.d = (double)v;
            frame->push(res);
            break;
        }
        case OP_D2I: {
            double v = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::INT; res.val.i = (int32_t)v;
            frame->push(res);
            break;
        }
        case OP_D2L: {
            double v = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::LONG; res.val.l = (int64_t)v;
            frame->push(res);
            break;
        }
        case OP_D2F: {
            double v = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::FLOAT; res.val.f = (float)v;
            frame->push(res);
            break;
        }
        case OP_I2B: {
            int32_t v = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = (int8_t)v;
            frame->push(res);
            break;
        }
        case OP_I2C: {
            int32_t v = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = (uint16_t)v;
            frame->push(res);
            break;
        }
        case OP_I2S: {
            int32_t v = frame->pop().val.i;
            JavaValue res; res.type = JavaValue::INT; res.val.i = (int16_t)v;
            frame->push(res);
            break;
        }

        // --- Comparisons ---
        case OP_LCMP: {
            int64_t v2 = frame->pop().val.l;
            int64_t v1 = frame->pop().val.l;
            JavaValue res; res.type = JavaValue::INT;
            if (v1 > v2) res.val.i = 1;
            else if (v1 < v2) res.val.i = -1;
            else res.val.i = 0;
            frame->push(res);
            break;
        }
        case OP_FCMPL: {
            float v2 = frame->pop().val.f;
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::INT;
            if (std::isnan(v1) || std::isnan(v2)) res.val.i = -1;
            else if (v1 > v2) res.val.i = 1;
            else if (v1 < v2) res.val.i = -1;
            else res.val.i = 0;
            frame->push(res);
            break;
        }
        case OP_FCMPG: {
            float v2 = frame->pop().val.f;
            float v1 = frame->pop().val.f;
            JavaValue res; res.type = JavaValue::INT;
            if (std::isnan(v1) || std::isnan(v2)) res.val.i = 1;
            else if (v1 > v2) res.val.i = 1;
            else if (v1 < v2) res.val.i = -1;
            else res.val.i = 0;
            frame->push(res);
            break;
        }
        case OP_DCMPL: {
            double v2 = frame->pop().val.d;
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::INT;
            if (std::isnan(v1) || std::isnan(v2)) res.val.i = -1;
            else if (v1 > v2) res.val.i = 1;
            else if (v1 < v2) res.val.i = -1;
            else res.val.i = 0;
            frame->push(res);
            break;
        }
        case OP_DCMPG: {
            double v2 = frame->pop().val.d;
            double v1 = frame->pop().val.d;
            JavaValue res; res.type = JavaValue::INT;
            if (std::isnan(v1) || std::isnan(v2)) res.val.i = 1;
            else if (v1 > v2) res.val.i = 1;
            else if (v1 < v2) res.val.i = -1;
            else res.val.i = 0;
            frame->push(res);
            break;
        }

        case OP_IINC: {
            uint8_t index = codeReader.readU1();
            int8_t constVal = (int8_t)codeReader.readU1();
            JavaValue val = frame->getLocal(index);
            val.val.i += constVal;
            frame->setLocal(index, val);
            break;
        }

        case OP_IFEQ: {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val = frame->pop().val.i;
             if (val == 0) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IFNE: {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val = frame->pop().val.i;
             if (val != 0) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IFLT: {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val = frame->pop().val.i;
             if (val < 0) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IFGE: {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val = frame->pop().val.i;
             if (val >= 0) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IFGT: {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val = frame->pop().val.i;
             if (val > 0) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IFLE: {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val = frame->pop().val.i;
             if (val <= 0) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IF_ICMPEQ: {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val2 = frame->pop().val.i;
             int32_t val1 = frame->pop().val.i;
             if (val1 == val2) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IF_ICMPNE: {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val2 = frame->pop().val.i;
             int32_t val1 = frame->pop().val.i;
             if (val1 != val2) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IF_ICMPLT: {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val2 = frame->pop().val.i;
             int32_t val1 = frame->pop().val.i;
             
             if (val1 < val2) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IF_ICMPGE: {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val2 = frame->pop().val.i;
             int32_t val1 = frame->pop().val.i;

             if (val1 >= val2) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IF_ICMPGT: {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val2 = frame->pop().val.i;
             int32_t val1 = frame->pop().val.i;

             if (val1 > val2) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IF_ICMPLE: {
             int16_t offset = (int16_t)codeReader.readU2();
             int32_t val2 = frame->pop().val.i;
             int32_t val1 = frame->pop().val.i;

             if (val1 <= val2) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IF_ACMPEQ: {
             int16_t offset = (int16_t)codeReader.readU2();
             JavaValue val2 = frame->pop();
             JavaValue val1 = frame->pop();
             if (val1.val.ref == val2.val.ref) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IF_ACMPNE: {
             int16_t offset = (int16_t)codeReader.readU2();
             JavaValue val2 = frame->pop();
             JavaValue val1 = frame->pop();
             if (val1.val.ref != val2.val.ref) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IFNULL: {
             int16_t offset = (int16_t)codeReader.readU2();
             JavaValue val = frame->pop();
             if (val.val.ref == nullptr) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }
        case OP_IFNONNULL: {
             int16_t offset = (int16_t)codeReader.readU2();
             JavaValue val = frame->pop();
             if (val.val.ref != nullptr) {
                 codeReader.seek(codeReader.tell() + offset - 3); 
             }
             break;
        }

        case OP_TABLESWITCH: {
            size_t pc = codeReader.tell() - 1;
            size_t padding = (4 - (pc + 1) % 4) % 4;
            for (size_t i = 0; i < padding; i++) codeReader.readU1();
            
            int32_t defaultOffset = (int32_t)codeReader.readU4();
            int32_t low = (int32_t)codeReader.readU4();
            int32_t high = (int32_t)codeReader.readU4();
            
            int32_t index = frame->pop().val.i;
            
            if (index < low || index > high) {
                codeReader.seek(pc + defaultOffset);
            } else {
                size_t jumpTableStart = codeReader.tell();
                size_t entryOffset = (index - low) * 4;
                codeReader.seek(jumpTableStart + entryOffset);
                int32_t offset = (int32_t)codeReader.readU4();
                codeReader.seek(pc + offset);
            }
            break;
        }
        
        case OP_LOOKUPSWITCH: {
            size_t pc = codeReader.tell() - 1;
            size_t padding = (4 - (pc + 1) % 4) % 4;
            for (size_t i = 0; i < padding; i++) codeReader.readU1();
            
            int32_t defaultOffset = (int32_t)codeReader.readU4();
            int32_t npairs = (int32_t)codeReader.readU4();
            
            int32_t key = frame->pop().val.i;
            int32_t targetOffset = defaultOffset;
            
            for (int i = 0; i < npairs; i++) {
                int32_t match = (int32_t)codeReader.readU4();
                int32_t offset = (int32_t)codeReader.readU4();
                if (match == key) {
                    targetOffset = offset;
                    break;
                }
            }
            codeReader.seek(pc + targetOffset);
            break;
        }

        case OP_GOTO: {
             int16_t offset = (int16_t)codeReader.readU2();
             codeReader.seek(codeReader.tell() + offset - 3); 
             break;
        }
        case OP_GOTO_W: {
             int32_t offset = (int32_t)codeReader.readU4();
             codeReader.seek(codeReader.tell() + offset - 5); 
             break;
        }

        // --- Arrays ---
        case OP_NEWARRAY: {
            uint8_t atype = codeReader.readU1();
            int32_t count = frame->pop().val.i;
            if (count < 0) throw std::runtime_error("NegativeArraySizeException");
            
            // Allocate array
            // We use JavaObject with null class for now, and fields as storage
            JavaObject* obj = new JavaObject(nullptr); // HeapManager should handle this
            // But HeapManager::allocate takes JavaClass.
            // Let's modify HeapManager or just new it here (leak?)
            // We should add Array support to HeapManager later.
            // For now:
            obj->fields.resize(count);
            // atype determines element size, but we use 64-bit slots.
            
            JavaValue val;
            val.type = JavaValue::REFERENCE;
            val.val.ref = obj;
            frame->push(val);
            break;
        }

        case OP_ANEWARRAY: {
            uint16_t index = codeReader.readU2();
            auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[index]);
            auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
            
            int32_t count = frame->pop().val.i;
            if (count < 0) throw std::runtime_error("NegativeArraySizeException");
            
            // Allocate array
            // We use JavaObject with null class for now, and fields as storage
            JavaObject* obj = new JavaObject(nullptr);
            obj->fields.resize(count);
            
            // Set a marker to identify this as an array
            // We'll use cls == nullptr as the array marker
            
            JavaValue val;
            val.type = JavaValue::REFERENCE;
            val.val.ref = obj;
            frame->push(val);
            break;
        }
        
        case OP_IASTORE: {
            JavaValue val = frame->pop();
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            arr->fields[index] = val.val.i;
            break;
        }

        case OP_CASTORE: {
            JavaValue val = frame->pop();
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            arr->fields[index] = val.val.i;
            break;
        }

        case OP_AASTORE: {
            JavaValue val = frame->pop();
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            arr->fields[index] = (int64_t)val.val.ref;
            break;
        }

        case OP_LASTORE: {
            JavaValue val = frame->pop();
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            arr->fields[index] = val.val.l;
            break;
        }

        case OP_FASTORE: {
            JavaValue val = frame->pop();
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            int32_t bits;
            memcpy(&bits, &val.val.f, sizeof(float));
            arr->fields[index] = bits;
            break;
        }

        case OP_DASTORE: {
            JavaValue val = frame->pop();
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            int64_t bits;
            memcpy(&bits, &val.val.d, sizeof(double));
            arr->fields[index] = bits;
            break;
        }

        case OP_BASTORE: {
            JavaValue val = frame->pop();
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            arr->fields[index] = val.val.i;
            break;
        }

        case OP_SASTORE: {
            JavaValue val = frame->pop();
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            arr->fields[index] = val.val.i;
            break;
        }
        
        case OP_IALOAD: {
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            JavaValue val;
            val.type = JavaValue::INT;
            val.val.i = (int32_t)arr->fields[index];
            frame->push(val);
            break;
        }

        case OP_CALOAD: {
            int32_t index = frame->pop().val.i;
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            if (index < 0 || index >= arr->fields.size()) throw std::runtime_error("ArrayIndexOutOfBoundsException");
            
            JavaValue val;
            val.type = JavaValue::INT;
            val.val.i = (uint16_t)arr->fields[index];
            frame->push(val);
            break;
        }

        case OP_LALOAD: {
            int32_t index = frame->pop().val.i;
            JavaValue arrayRef = frame->pop();
            if (arrayRef.val.ref == nullptr) {
                throw std::runtime_error("NullPointerException");
            }
            JavaObject* arrayObj = (JavaObject*)arrayRef.val.ref;
            if (index < 0 || index >= arrayObj->fields.size()) {
                throw std::runtime_error("ArrayIndexOutOfBoundsException");
            }
            JavaValue val; val.type = JavaValue::LONG; val.val.l = arrayObj->fields[index];
            frame->push(val);
            break;
        }
        case OP_FALOAD: {
            int32_t index = frame->pop().val.i;
            JavaValue arrayRef = frame->pop();
            if (arrayRef.val.ref == nullptr) {
                throw std::runtime_error("NullPointerException");
            }
            JavaObject* arrayObj = (JavaObject*)arrayRef.val.ref;
            if (index < 0 || index >= arrayObj->fields.size()) {
                throw std::runtime_error("ArrayIndexOutOfBoundsException");
            }
            // Float is stored as bits in int64 field
            JavaValue val; val.type = JavaValue::FLOAT; 
            // Reinterpret bits
            int32_t bits = (int32_t)arrayObj->fields[index];
            memcpy(&val.val.f, &bits, sizeof(float));
            frame->push(val);
            break;
        }
        case OP_DALOAD: {
            int32_t index = frame->pop().val.i;
            JavaValue arrayRef = frame->pop();
            if (arrayRef.val.ref == nullptr) {
                throw std::runtime_error("NullPointerException");
            }
            JavaObject* arrayObj = (JavaObject*)arrayRef.val.ref;
            if (index < 0 || index >= arrayObj->fields.size()) {
                throw std::runtime_error("ArrayIndexOutOfBoundsException");
            }
            JavaValue val; val.type = JavaValue::DOUBLE; 
            double d;
            memcpy(&d, &arrayObj->fields[index], sizeof(double));
            val.val.d = d;
            frame->push(val);
            break;
        }
        case OP_AALOAD: {
            int32_t index = frame->pop().val.i;
            JavaValue arrayRef = frame->pop();
            if (arrayRef.val.ref == nullptr) {
                throw std::runtime_error("NullPointerException");
            }
            JavaObject* arrayObj = (JavaObject*)arrayRef.val.ref;
            if (index < 0 || index >= arrayObj->fields.size()) {
                throw std::runtime_error("ArrayIndexOutOfBoundsException");
            }
            JavaValue val; val.type = JavaValue::REFERENCE; val.val.ref = (void*)arrayObj->fields[index];
            frame->push(val);
            break;
        }
        case OP_BALOAD: {
            int32_t index = frame->pop().val.i;
            JavaValue arrayRef = frame->pop();
            if (arrayRef.val.ref == nullptr) {
                throw std::runtime_error("NullPointerException");
            }
            JavaObject* arrayObj = (JavaObject*)arrayRef.val.ref;
            if (index < 0 || index >= arrayObj->fields.size()) {
                throw std::runtime_error("ArrayIndexOutOfBoundsException");
            }
            JavaValue val; val.type = JavaValue::INT; val.val.i = (int8_t)arrayObj->fields[index];
            frame->push(val);
            break;
        }
        case OP_SALOAD: {
            int32_t index = frame->pop().val.i;
            JavaValue arrayRef = frame->pop();
            if (arrayRef.val.ref == nullptr) {
                throw std::runtime_error("NullPointerException");
            }
            JavaObject* arrayObj = (JavaObject*)arrayRef.val.ref;
            if (index < 0 || index >= arrayObj->fields.size()) {
                throw std::runtime_error("ArrayIndexOutOfBoundsException");
            }
            JavaValue val; val.type = JavaValue::INT; val.val.i = (int16_t)arrayObj->fields[index];
            frame->push(val);
            break;
        }

        case OP_ARRAYLENGTH: {
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            JavaValue val;
            val.type = JavaValue::INT;
            val.val.i = arr->fields.size();
            frame->push(val);
            break;
        }

        // --- Objects ---
        case OP_NEW: {
            uint16_t index = codeReader.readU2();
            auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[index]);
            auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
            
            // std::cout << "[OP_NEW] Creating instance of: " << className->bytes << std::endl;
            
            // Validate class name before resolving
            if (!isValidClassName(className->bytes)) {
                throw std::runtime_error("Invalid class name in OP_NEW: " + className->bytes);
            }
            
            auto cls = resolveClass(className->bytes);
            if (!cls) {
                throw std::runtime_error("Could not find class: " + className->bytes);
            }
            
            initializeClass(cls);
            
            JavaObject* obj = HeapManager::getInstance().allocate(cls);
            JavaValue val;
            val.type = JavaValue::REFERENCE;
            val.val.ref = obj;
            frame->push(val);
            // std::cout << "[OP_NEW] Created instance at: " << obj << std::endl;
            break;
        }

        case OP_ATHROW: {
            JavaValue exceptionVal = frame->pop();
            if (exceptionVal.val.ref == nullptr) {
                throw std::runtime_error("NullPointerException in ATHROW");
            }
            
            JavaObject* exObj = (JavaObject*)exceptionVal.val.ref;
            std::string exClass = "Unknown";
            if (exObj->cls) exClass = exObj->cls->name;
            
            // TODO: Implement exception handling (search for handler in stack)
            // For now, treat as unhandled and throw runtime error
            throw std::runtime_error("Unhandled Java Exception: " + exClass);
            break;
        }

        case OP_CHECKCAST: {
            uint16_t index = codeReader.readU2();
            auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[index]);
            auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
            
            JavaValue objVal = frame->peek();
            if (objVal.val.ref == nullptr) {
                break;
            }
            
            JavaObject* obj = static_cast<JavaObject*>(objVal.val.ref);
            if (!obj->cls) {
                 // Array
                 if (className->bytes == "java/lang/Object") break;
                 // Simple name check for arrays
                 // TODO: Better array check
                 break; 
            }
            
            bool found = false;
            std::shared_ptr<JavaClass> current = obj->cls;
            while (current) {
                if (current->name == className->bytes) {
                    found = true;
                    break;
                }
                current = current->superClass;
            }
            
            if (!found) {
                // Also check interfaces?
                // For now, just throw if not found
                // Actually, let's log and throw
                LOG_ERROR("ClassCastException: " + obj->cls->name + " cannot be cast to " + className->bytes);
                throw std::runtime_error("ClassCastException: " + obj->cls->name + " to " + className->bytes);
            }
            break;
        }

        case OP_INSTANCEOF: {
            uint16_t index = codeReader.readU2();
            auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[index]);
            auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
            
            JavaValue objVal = frame->pop();
            if (objVal.val.ref == nullptr) {
                frame->push(JavaValue{JavaValue::INT, {.i = 0}});
                break;
            }
            
            JavaObject* obj = static_cast<JavaObject*>(objVal.val.ref);
            bool isInstance = false;
            
            if (!obj->cls) {
                 if (className->bytes == "java/lang/Object") isInstance = true;
            } else {
                 std::shared_ptr<JavaClass> current = obj->cls;
                 while (current) {
                     if (current->name == className->bytes) {
                         isInstance = true;
                         break;
                     }
                     current = current->superClass;
                 }
            }
            frame->push(JavaValue{JavaValue::INT, {.i = isInstance ? 1 : 0}});
            break;
        }

        case OP_GETFIELD: {
            uint16_t index = codeReader.readU2();
            auto fieldRef = std::dynamic_pointer_cast<ConstantRef>(frame->classFile->constant_pool[index]);
            auto nameAndType = std::dynamic_pointer_cast<ConstantNameAndType>(frame->classFile->constant_pool[fieldRef->name_and_type_index]);
            auto name = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->name_index]);
            auto descriptor = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->descriptor_index]);
            
            JavaValue objVal = frame->pop();
            if (objVal.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* obj = static_cast<JavaObject*>(objVal.val.ref);
            
            // Special handling for array length field
            if (obj->cls == nullptr && name->bytes == "length") {
                JavaValue fieldVal;
                fieldVal.type = JavaValue::INT;
                fieldVal.val.i = obj->fields.size();
                frame->push(fieldVal);
                break;
            }
            
            // Look up offset
            std::string key = name->bytes + "|" + descriptor->bytes;
            auto it = obj->cls->fieldOffsets.find(key);
            if (it == obj->cls->fieldOffsets.end()) {
                throw std::runtime_error("Field not found: " + key);
            }
            
            JavaValue fieldVal;
            char typeChar = descriptor->bytes[0];
            if (typeChar == 'L' || typeChar == '[') {
                fieldVal.type = JavaValue::REFERENCE;
                fieldVal.val.ref = (void*)obj->fields[it->second];
            } else if (typeChar == 'J') {
                fieldVal.type = JavaValue::LONG;
                fieldVal.val.l = obj->fields[it->second];
            } else if (typeChar == 'D') {
                fieldVal.type = JavaValue::DOUBLE;
                // Copy bits
                int64_t bits = obj->fields[it->second];
                memcpy(&fieldVal.val.d, &bits, sizeof(double));
            } else if (typeChar == 'F') {
                 fieldVal.type = JavaValue::FLOAT;
                 int32_t bits = (int32_t)obj->fields[it->second];
                 memcpy(&fieldVal.val.f, &bits, sizeof(float));
            } else {
                 fieldVal.type = JavaValue::INT;
                 fieldVal.val.i = (int32_t)obj->fields[it->second];
            }
            frame->push(fieldVal);
            break;
        }
        
        case OP_PUTFIELD: {
            uint16_t index = codeReader.readU2();
            auto fieldRef = std::dynamic_pointer_cast<ConstantRef>(frame->classFile->constant_pool[index]);
            if (!fieldRef) throw std::runtime_error("Invalid field ref");
            
            auto nameAndType = std::dynamic_pointer_cast<ConstantNameAndType>(frame->classFile->constant_pool[fieldRef->name_and_type_index]);
            if (!nameAndType) throw std::runtime_error("Invalid nameAndType");
            
            auto name = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->name_index]);
            if (!name) throw std::runtime_error("Invalid field name");
            auto descriptor = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->descriptor_index]);

            JavaValue val = frame->pop();
            JavaValue objVal = frame->pop();
            
            if (objVal.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* obj = static_cast<JavaObject*>(objVal.val.ref);
            
            if (!obj) throw std::runtime_error("Object is null");
            
            // Special handling for array length field (read-only, but we'll ignore writes)
            if (obj->cls == nullptr && name->bytes == "length") {
                // Array length is read-only, ignore the write
                break;
            }
            
            if (!obj->cls) throw std::runtime_error("Object class is null");
            
            // Look up offset
            std::string key = name->bytes + "|" + descriptor->bytes;
            auto it = obj->cls->fieldOffsets.find(key);
            if (it == obj->cls->fieldOffsets.end()) {
                throw std::runtime_error("Field not found: " + key);
            }
            
            if (it->second >= obj->fields.size()) {
                 throw std::runtime_error("Field offset out of bounds");
            }
            
            char typeChar = descriptor->bytes[0];
            if (typeChar == 'L' || typeChar == '[') {
                obj->fields[it->second] = (int64_t)val.val.ref;
            } else if (typeChar == 'J') {
                obj->fields[it->second] = val.val.l;
            } else if (typeChar == 'D') {
                int64_t bits;
                memcpy(&bits, &val.val.d, sizeof(double));
                obj->fields[it->second] = bits;
            } else if (typeChar == 'F') {
                int32_t bits;
                memcpy(&bits, &val.val.f, sizeof(float));
                obj->fields[it->second] = (int64_t)bits;
            } else {
                obj->fields[it->second] = val.val.i;
            }
            break;
        }

        case OP_GETSTATIC: {
            uint16_t index = codeReader.readU2();
            auto ref = std::dynamic_pointer_cast<ConstantRef>(frame->classFile->constant_pool[index]);
            if (ref) {
                 auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[ref->class_index]);
                 auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
                 
                 // Validate class name before resolving
                 if (!isValidClassName(className->bytes)) {
                     throw std::runtime_error("Invalid class name in GETSTATIC: " + className->bytes);
                 }
                 
                 // Regular GETSTATIC
                 auto nameAndType = std::dynamic_pointer_cast<ConstantNameAndType>(frame->classFile->constant_pool[ref->name_and_type_index]);
                 auto name = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->name_index]);
                 
                 auto cls = resolveClass(className->bytes);
                 if (!cls) throw std::runtime_error("Class not found: " + className->bytes);
                 
                 // Initialize class if needed
                 initializeClass(cls);
                 
                 auto descriptor = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->descriptor_index]);
                 auto it = cls->staticFields.find(name->bytes + "|" + descriptor->bytes);
                 if (it == cls->staticFields.end()) {
                     // Default value 0/null
                     JavaValue val;
                     // For static fields, we need to determine type from descriptor
                     if (descriptor && descriptor->bytes[0] == 'L') {
                         // Reference type
                         val.type = JavaValue::REFERENCE;
                         val.val.ref = nullptr;
                     } else {
                         // Primitive type
                         val.type = JavaValue::INT;
                         val.val.i = 0;
                     }
                     frame->push(val);
                 } else {
                     JavaValue val;
                     // Determine type from descriptor
                     auto descriptor = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->descriptor_index]);
                     if (descriptor && descriptor->bytes[0] == 'L') {
                         // Reference type
                         val.type = JavaValue::REFERENCE;
                         val.val.ref = (void*)it->second;
                     } else {
                         // Primitive type
                         val.type = JavaValue::LONG;
                         val.val.l = it->second;
                     }
                     frame->push(val);
                 }
                 break;
            }
            std::cerr << "Unsupported GETSTATIC index: " << index << std::endl;
            break;
        }

        case OP_PUTSTATIC: {
            uint16_t index = codeReader.readU2();
            auto ref = std::dynamic_pointer_cast<ConstantRef>(frame->classFile->constant_pool[index]);
            if (ref) {
                 auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[ref->class_index]);
                 auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
                 auto nameAndType = std::dynamic_pointer_cast<ConstantNameAndType>(frame->classFile->constant_pool[ref->name_and_type_index]);
                 auto name = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->name_index]);
                 auto descriptor = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->descriptor_index]);
                 
                 // Validate class name before resolving
                 if (!isValidClassName(className->bytes)) {
                     throw std::runtime_error("Invalid class name in PUTSTATIC: " + className->bytes);
                 }
                 
                 auto cls = resolveClass(className->bytes);
                 if (!cls) throw std::runtime_error("Class not found: " + className->bytes);
                 
                 // Initialize class if needed
                 initializeClass(cls);
                 
                 JavaValue val = frame->pop();
                 // Store value - handle different types appropriately
                 std::string key = name->bytes + "|" + descriptor->bytes;
                 if (val.type == JavaValue::REFERENCE) {
                     cls->staticFields[key] = (int64_t)val.val.ref;
                 } else {
                     cls->staticFields[key] = val.val.l;
                 }
                 break;
            }
            std::cerr << "Unsupported PUTSTATIC index: " << index << std::endl;
            break;
        }
        
        case OP_INVOKESPECIAL: 
        case OP_INVOKESTATIC: {
             uint16_t index = codeReader.readU2();
             auto methodRef = std::dynamic_pointer_cast<ConstantRef>(frame->classFile->constant_pool[index]);
             auto nameAndType = std::dynamic_pointer_cast<ConstantNameAndType>(frame->classFile->constant_pool[methodRef->name_and_type_index]);
             auto name = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->name_index]);
             auto descriptor = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->descriptor_index]);
             
             auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[methodRef->class_index]);
             auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);

             // Validate class name before resolving
             if (!isValidClassName(className->bytes)) {
                 throw std::runtime_error("Invalid class name in INVOKE: " + className->bytes);
             }

             bool isStatic = (opcode == OP_INVOKESTATIC);

             // std::cout << "[OP_" << (isStatic ? "INVOKESTATIC" : "INVOKESPECIAL") << "] " << className->bytes << "." << name->bytes << descriptor->bytes << std::endl;

             // Calculate arg count from descriptor
             // Descriptor format: (arg1typearg2type...)returntype
             int argCount = 0;
             bool inArgs = false;
             size_t i = 0;
             while (i < descriptor->bytes.length()) {
                 char c = descriptor->bytes[i];
                 if (c == '(') {
                     inArgs = true;
                     i++;
                     continue;
                 }
                 if (c == ')') {
                     break;
                 }
                 if (inArgs) {
                     if (c == 'L') {
                         // Reference type, skip until ';' and count as 1 arg
                         argCount++;
                         i++;
                         while (i < descriptor->bytes.length() && descriptor->bytes[i] != ';') {
                             i++;
                         }
                         // Skip the ';' itself
                         i++;
                     } else if (c == '[') {
                         // Array type, continue to next char to get the element type
                         i++;
                     } else {
                         // Primitive type: I, J, F, D, Z, B, C, S
                         argCount++;
                         i++;
                     }
                 } else {
                     i++;
                 }
             }
             
             // std::cout << "[Interpreter] Descriptor: " << descriptor->bytes << ", argCount: " << argCount << std::endl;
             
             if (!isStatic) argCount++; // 'this'

             bool isConstructor = (name->bytes == "<init>");

             // Resolve class first
             auto cls = resolveClass(className->bytes);
             if (!cls) throw std::runtime_error("Class not found: " + className->bytes);
             
             if (isStatic) initializeClass(cls);

             // Find method
             bool found = false;
             for (const auto& m : cls->rawFile->methods) {
                 auto mName = std::dynamic_pointer_cast<ConstantUtf8>(cls->rawFile->constant_pool[m.name_index]);
                 auto mDesc = std::dynamic_pointer_cast<ConstantUtf8>(cls->rawFile->constant_pool[m.descriptor_index]);
                 
                 if (mName->bytes == name->bytes && mDesc->bytes == descriptor->bytes) {
                     // std::cout << "[Interpreter] Found method: " << className->bytes << "." << name->bytes << descriptor->bytes << " flags=" << std::hex << m.access_flags << std::dec << std::endl;
                     
                     if (m.access_flags & 0x0100) { // ACC_NATIVE
                         // std::cout << "[Interpreter] Native method call" << std::endl;
                         auto nativeFunc = NativeRegistry::getInstance().getNative(className->bytes, name->bytes, descriptor->bytes);
                         if (nativeFunc) {
                             nativeFunc(frame);
                         } else {
                             std::cerr << "UnsatisfiedLinkError: " << className->bytes << "." << name->bytes << descriptor->bytes << std::endl;
                             throw std::runtime_error("UnsatisfiedLinkError: " + className->bytes + "." + name->bytes + descriptor->bytes);
                         }
                     } else {
                         // Java method call
                         // std::cout << "[Interpreter] Java method call, creating frame with " << argCount << " args" << std::endl;
                         
                         // Special handling for java/lang/Object.<init>
                         if (cls->name == "java/lang/Object" && name->bytes == "<init>") {
                             // std::cout << "[Interpreter] Skipping java/lang/Object.<init> (no-op)" << std::endl;
                             found = true;
                             break;
                         }
                         
                         auto newFrame = std::make_shared<StackFrame>(m, cls->rawFile);
                         
                         // Pop args from stack in reverse order
                         std::vector<JavaValue> args;
                         for (int i = 0; i < argCount; i++) {
                             args.push_back(frame->pop());
                         }
                         
                         // Set locals
                        int localIndex = 0;
                        for (int i = 0; i < argCount; i++) {
                            JavaValue& val = args[argCount - 1 - i];
                            newFrame->setLocal(localIndex, val);
                            localIndex++;
                            if (val.type == JavaValue::LONG || val.type == JavaValue::DOUBLE) {
                                localIndex++;
                            }
                        }
                         
                         // Constructor super call handling (same as before)
                         if (isConstructor && cls->superClass && cls->superClass->name != "java/lang/Object") {
                             bool superFound = false;
                             for (const auto& superMethod : cls->superClass->rawFile->methods) {
                                 auto superName = std::dynamic_pointer_cast<ConstantUtf8>(cls->superClass->rawFile->constant_pool[superMethod.name_index]);
                                 auto superDesc = std::dynamic_pointer_cast<ConstantUtf8>(cls->superClass->rawFile->constant_pool[superMethod.descriptor_index]);
                                 if (superName->bytes == "<init>" && superDesc->bytes == "()V") {
                                     auto superFrame = std::make_shared<StackFrame>(superMethod, cls->superClass->rawFile);
                                     superFrame->setLocal(0, newFrame->getLocal(0));
                                     execute(superFrame);
                                     superFound = true;
                                     break;
                                 }
                             }
                         }
                         
                         auto ret = execute(newFrame);
                         
                         if (descriptor->bytes.back() != 'V') {
                             if (ret) {
                                 frame->push(*ret);
                             }
                         }
                     }
                     found = true;
                     break;
                 }
             }
             
             if (!found) {
                 // Check if it's a native method NOT in class file (e.g. system hooks)
                 // This is fallback for methods not in rt.jar but registered
                 auto nativeFunc = NativeRegistry::getInstance().getNative(className->bytes, name->bytes, descriptor->bytes);
                 if (nativeFunc) {
                      // std::cout << "[Interpreter] Calling registered native (not in class): " << className->bytes << "." << name->bytes << std::endl;
                      nativeFunc(frame);
                 } else {
                      std::cerr << "Method not found: " << className->bytes << "." << name->bytes << std::endl;
                      throw std::runtime_error("Method not found: " + className->bytes + "." + name->bytes);
                 }
             }

             break;
        }

        case OP_INVOKEVIRTUAL: {
            uint16_t index = codeReader.readU2();
            
            // Need to know how many args to pop!
            auto methodRef = std::dynamic_pointer_cast<ConstantRef>(frame->classFile->constant_pool[index]);
            auto nameAndType = std::dynamic_pointer_cast<ConstantNameAndType>(frame->classFile->constant_pool[methodRef->name_and_type_index]);
            auto name = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->name_index]);
            auto descriptor = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->descriptor_index]);
            
            // Better arg counting
            int argCount = 0;
            bool parsingObj = false;
            for (size_t i = 1; i < descriptor->bytes.length(); ++i) { // Skip '('
                char c = descriptor->bytes[i];
                if (c == ')') break;
                if (parsingObj) {
                    if (c == ';') parsingObj = false;
                    continue;
                }
                if (c == 'L') {
                    parsingObj = true;
                    argCount++;
                } else if (c == '[') {
                    // Array, skip next char? 
                    // Arrays are objects, take 1 slot.
                    // But next char is type.
                    // e.g. [I, [[I, [Ljava/lang/String;
                    // Just continue, counting happens on type.
                } else {
                    // I, Z, B, C, S, F, J, D
                    argCount++;
                }
            }

            auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[methodRef->class_index]);
            auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
            
            // Validate class name before resolving
            if (!isValidClassName(className->bytes)) {
                throw std::runtime_error("Invalid class name in INVOKEVIRTUAL: " + className->bytes);
            }
            
            // std::cout << "[INVOKEVIRTUAL] Calling " << className->bytes << "." << name->bytes << descriptor->bytes << " with " << argCount << " args" << std::endl;
            
            std::vector<JavaValue> args;
            for(int i=0; i<argCount; ++i) args.push_back(frame->pop());
            // args[0] is last argument.
            
            JavaValue obj = frame->pop();
            
            if (obj.val.ref == (void*)0xDEADBEEF) {
                // System.out.println
                if (name->bytes == "println" && !args.empty()) {
                    auto& arg = args[argCount-1]; // First argument
                    if (arg.type == JavaValue::REFERENCE && !arg.strVal.empty()) {
                        std::cout << "JVM OUTPUT: " << arg.strVal << std::endl;
                    } else if (arg.type == JavaValue::INT) {
                         std::cout << "JVM OUTPUT: " << arg.val.i << std::endl;
                    }
                } else if (name->bytes == "toString") {
                    JavaValue ret;
                    ret.type = JavaValue::REFERENCE;
                    ret.strVal = "System.out";
                    frame->push(ret);
                }
            } else {
                 // Regular virtual call (e.g. Point.print())
                 // Resolve class from object
                 if (obj.val.ref == nullptr) throw std::runtime_error("NullPointerException");
                 
                 JavaObject* javaObj = static_cast<JavaObject*>(obj.val.ref);
                 auto cls = javaObj->cls;
                 
                 // std::cout << "[INVOKEVIRTUAL] Object class: " << cls->name << std::endl;
                /*
                if (name->bytes == "setFullScreenMode" || name->bytes == "getWidth") {
                    std::cout << "Debug: Invoking " << name->bytes << " on object of type " << (cls ? cls->name : "null") << std::endl;
                }
                */
                 
                 // Find method
                 bool found = false;
                 std::shared_ptr<JavaClass> currentClass = cls;

                 while (currentClass != nullptr) {
                     for (const auto& m : currentClass->rawFile->methods) {
                         auto mName = std::dynamic_pointer_cast<ConstantUtf8>(currentClass->rawFile->constant_pool[m.name_index]);
                         auto mDesc = std::dynamic_pointer_cast<ConstantUtf8>(currentClass->rawFile->constant_pool[m.descriptor_index]);
                         
                         if (mName->bytes == name->bytes && mDesc->bytes == descriptor->bytes) {
                             // Check if native
                             // std::cout << "[INVOKEVIRTUAL] Method flags: " << std::hex << m.access_flags << std::dec << std::endl;
                             // FORCE Native for StringBuffer to avoid mixed Java/Native issues with mock class
                             if ((m.access_flags & 0x0100) || currentClass->name == "java/lang/StringBuffer") { // ACC_NATIVE
                                 // Restore stack for native call
                                 frame->push(obj);
                                 for (int i = argCount - 1; i >= 0; i--) {
                                     frame->push(args[i]);
                                 }
                                 
                                 auto nativeFunc = NativeRegistry::getInstance().getNative(currentClass->name, name->bytes, descriptor->bytes);
                                 if (nativeFunc) {
                                     nativeFunc(frame);
                                 } else {
                                     std::cerr << "UnsatisfiedLinkError (virtual): " << currentClass->name << "." << name->bytes << descriptor->bytes << std::endl;
                                 }
                             } else {
                                 auto newFrame = std::make_shared<StackFrame>(m, currentClass->rawFile);
                                 newFrame->setLocal(0, obj); // this
                                 
                                 // Args are popped in reverse: last arg is at args[0]
                                // setLocal(1) should be first arg.
                                // args[argCount-1] is first arg.
                                int localIndex = 1;
                                for(int i=0; i<argCount; ++i) {
                                    JavaValue& val = args[argCount - 1 - i];
                                    newFrame->setLocal(localIndex, val);
                                    localIndex++;
                                    if (val.type == JavaValue::LONG || val.type == JavaValue::DOUBLE) {
                                        localIndex++;
                                    }
                                }
                                 
                                 auto ret = execute(newFrame);
                                 
                                 // Push return value if any
                                 if (descriptor->bytes.back() != 'V') {
                                     if (ret) {
                                         frame->push(*ret);
                                     }
                                 }
                             }
                             
                             found = true;
                             break;
                         }
                     }
                     
                     if (found) break;
                     
                     // Try superclass
                     if (currentClass->rawFile->super_class == 0) {
                         currentClass = nullptr;
                     } else {
                         auto superClassRef = std::dynamic_pointer_cast<ConstantClass>(currentClass->rawFile->constant_pool[currentClass->rawFile->super_class]);
                         auto superClassName = std::dynamic_pointer_cast<ConstantUtf8>(currentClass->rawFile->constant_pool[superClassRef->name_index]);
                         currentClass = resolveClass(superClassName->bytes);
                     }
                 }
                 
                 if (!found) {
                    std::string msg = "Method not found: " + cls->name + "." + name->bytes + descriptor->bytes;
                    throw std::runtime_error(msg);
                }
            }
            break;
        }
        
        case OP_INVOKEINTERFACE: {
            uint16_t index = codeReader.readU2();
            codeReader.readU1(); // count
            codeReader.readU1(); // 0
            
            auto methodRef = std::dynamic_pointer_cast<ConstantRef>(frame->classFile->constant_pool[index]);
            auto nameAndType = std::dynamic_pointer_cast<ConstantNameAndType>(frame->classFile->constant_pool[methodRef->name_and_type_index]);
            auto name = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->name_index]);
            auto descriptor = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->descriptor_index]);
            
            // Calculate arg count
            int argCount = 0;
            bool parsingObj = false;
            for (size_t i = 1; i < descriptor->bytes.length(); ++i) { // Skip '('
                char c = descriptor->bytes[i];
                if (c == ')') break;
                if (parsingObj) {
                    if (c == ';') parsingObj = false;
                    continue;
                }
                if (c == 'L') {
                    parsingObj = true;
                    argCount++;
                } else if (c == '[') {
                } else {
                    argCount++;
                }
            }

            auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[methodRef->class_index]);
            auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
            
            // std::cout << "[INVOKEINTERFACE] Calling " << className->bytes << "." << name->bytes << descriptor->bytes << " with " << argCount << " args" << std::endl;
            
            std::vector<JavaValue> args;
            for(int i=0; i<argCount; ++i) args.push_back(frame->pop());
            
            JavaValue obj = frame->pop();
            if (obj.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* javaObj = static_cast<JavaObject*>(obj.val.ref);
            auto cls = javaObj->cls;
            
             // Find method in object class
             bool found = false;
             std::shared_ptr<JavaClass> currentClass = cls;

             while (currentClass != nullptr) {
                 for (const auto& m : currentClass->rawFile->methods) {
                     auto mName = std::dynamic_pointer_cast<ConstantUtf8>(currentClass->rawFile->constant_pool[m.name_index]);
                     auto mDesc = std::dynamic_pointer_cast<ConstantUtf8>(currentClass->rawFile->constant_pool[m.descriptor_index]);
                     
                     if (mName->bytes == name->bytes && mDesc->bytes == descriptor->bytes) {
                        if (m.access_flags & 0x0100) { // ACC_NATIVE
                             // Restore stack for native call
                             frame->push(obj);
                             for (int i = argCount - 1; i >= 0; i--) {
                                 frame->push(args[i]);
                             }
                             
                             auto nativeFunc = NativeRegistry::getInstance().getNative(currentClass->name, name->bytes, descriptor->bytes);
                             if (nativeFunc) {
                                 nativeFunc(frame);
                             } else {
                                 std::cerr << "UnsatisfiedLinkError (interface): " << currentClass->name << "." << name->bytes << descriptor->bytes << std::endl;
                             }
                        } else {
                            auto newFrame = std::make_shared<StackFrame>(m, currentClass->rawFile);
                            newFrame->setLocal(0, obj); // this
                            
                            int localIndex = 1;
                            for(int i=0; i<argCount; ++i) {
                                JavaValue& val = args[argCount - 1 - i];
                                newFrame->setLocal(localIndex, val);
                                localIndex++;
                                if (val.type == JavaValue::LONG || val.type == JavaValue::DOUBLE) {
                                    localIndex++;
                                }
                            }
                            
                            auto ret = execute(newFrame);
                            
                            if (descriptor->bytes.back() != 'V') {
                                if (ret) {
                                    frame->push(*ret);
                                }
                            }
                        }
                         found = true;
                         break;
                     }
                 }
                 if (found) break;
                 
                 // Try superclass
                 if (currentClass->rawFile->super_class == 0) {
                     currentClass = nullptr;
                 } else {
                     auto superClassRef = std::dynamic_pointer_cast<ConstantClass>(currentClass->rawFile->constant_pool[currentClass->rawFile->super_class]);
                     auto superClassName = std::dynamic_pointer_cast<ConstantUtf8>(currentClass->rawFile->constant_pool[superClassRef->name_index]);
                     currentClass = resolveClass(superClassName->bytes);
                 }
             }
             if (!found) std::cerr << "Interface Method not found: " << name->bytes << std::endl;
             break;
        }

        case OP_MULTIANEWARRAY: {
            uint16_t index = codeReader.readU2();
            uint8_t dimensions = codeReader.readU1();
            
            std::vector<int32_t> counts;
            for (int i = 0; i < dimensions; i++) {
                counts.push_back(frame->pop().val.i);
            }
            std::reverse(counts.begin(), counts.end());
            
            // Helper lambda for recursive creation
            std::function<JavaObject*(int)> createArray;
            createArray = [&](int dimIndex) -> JavaObject* {
                if (dimIndex >= dimensions) return nullptr;
                
                int32_t count = counts[dimIndex];
                if (count < 0) throw std::runtime_error("NegativeArraySizeException");
                
                auto arrayObj = HeapManager::getInstance().allocate(nullptr);
                arrayObj->fields.resize(count, 0);
                
                if (dimIndex < dimensions - 1) {
                    for (int i = 0; i < count; i++) {
                         JavaObject* subArray = createArray(dimIndex + 1);
                         arrayObj->fields[i] = (int64_t)subArray;
                    }
                }
                
                return arrayObj;
            };
            
            JavaObject* result = createArray(0);
            
            JavaValue val;
            val.type = JavaValue::REFERENCE;
            val.val.ref = result;
            frame->push(val);
            break;
        }

        case OP_RETURN: {
            // std::cout << "[OP_RETURN] Returning from method" << std::endl;
            return false; // Stop execution
        }
        case OP_IRETURN:
        case OP_ARETURN:
        case OP_LRETURN:
        case OP_FRETURN:
        case OP_DRETURN: {
            returnVal = frame->pop();
            // std::cout << "[OP_IRETURN/OP_ARETURN/etc] Returning value" << std::endl;
            return false;
        }
        
        default:
            if (opcode == OP_L2I) std::cerr << "Error: Opcode is OP_L2I (" << (int)opcode << ") but fell to default!" << std::endl;
            if (opcode == OP_LCMP) std::cerr << "Error: Opcode is OP_LCMP (" << (int)opcode << ") but fell to default!" << std::endl;
            if (opcode == OP_LRETURN) std::cerr << "Error: Opcode is OP_LRETURN (" << (int)opcode << ") but fell to default!" << std::endl;
            std::cerr << "Unknown Opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
            break;
    }
    return true; // Continue execution
}

bool Interpreter::isValidClassName(const std::string& name) {
    // Empty string is invalid
    if (name.empty()) {
        return false;
    }
    // Field descriptor: L...; (e.g., Ljavax/microedition/media/Player;)
    if (name.size() >= 2 && name[0] == 'L' && name.back() == ';') {
        return false;
    }
    // Method descriptor: (...) (e.g., (Ljava/lang/String;)I)
    if (name[0] == '(') {
        return false;
    }
    // Single character (a-z, A-Z) - Allowed for obfuscated code
    // if (name.size() == 1) {
    //     return false;
    // }
    // Two characters (aa-zz, AA-ZZ) - Allowed for obfuscated code
    // if (name.size() == 2) {
    //     return false;
    // }
    // Special method names
    if (name == "<init>" || name == "<clinit>") {
        return false;
    }
    // Java keywords
    if (name == "void" || name == "int" || name == "long" ||
        name == "boolean" || name == "byte" ||
        name == "short" || name == "char" ||
        name == "float" || name == "double" ||
        name == "for" || name == "if" ||
        name == "else" || name == "while" ||
        name == "do" || name == "switch" ||
        name == "case" || name == "break" ||
        name == "continue" || name == "return" ||
        name == "new" || name == "instanceof" ||
        name == "class" || name == "interface" ||
        name == "extends" || name == "implements" ||
        name == "public" || name == "private" ||
        name == "protected" || name == "static" ||
        name == "final" || name == "abstract" ||
        name == "native" || name == "synchronized" ||
        name == "volatile" || name == "transient" ||
        name == "strictfp" || name == "throws" ||
        name == "throw" || name == "try" ||
        name == "catch" || name == "finally") {
        return false;
    }
    // Valid class names must:
    // 1. Start with '[' (array type) OR
    // 2. Contain '/' (package separator for fully qualified names) OR
    // 3. Simple class name (no package, just class name)
    if (name[0] == '[' || name.find('/') != std::string::npos) {
        return true;
    }
    // Simple class name: must start with letter, _, or $
    if (name.size() >= 1 && ((name[0] >= 'A' && name[0] <= 'Z') || (name[0] >= 'a' && name[0] <= 'z') || name[0] == '_' || name[0] == '$')) {
        bool valid = true;
        for (size_t i = 0; i < name.size(); i++) {
            char c = name[i];
            if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '$')) {
                valid = false;
                break;
            }
        }
        if (valid) {
            return true;
        }
    }
    // Everything else is invalid
    return false;
}

void Interpreter::registerClass(const std::string& className, std::shared_ptr<JavaClass> cls) {
    loadedClasses[className] = cls;
    LOG_DEBUG("[Interpreter] Registered class: " + className);
}

} // namespace core
} // namespace j2me
