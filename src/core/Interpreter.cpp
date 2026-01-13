#include "Interpreter.hpp"
#include "Opcodes.hpp"
#include "ClassParser.hpp"
#include "HeapManager.hpp"
#include "NativeRegistry.hpp"
#include <iostream>

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
        // Check system loader if available
        if (systemLoader && systemLoader->hasFile(path)) {
             auto data = systemLoader->getFile(path);
             if (data) {
                 try {
                     ClassParser parser;
                     auto rawFile = parser.parse(*data);
                     auto javaClass = std::make_shared<JavaClass>(rawFile);
                     
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
                     std::cerr << "Failed to parse system class " << className << ": " << e.what() << std::endl;
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
             addMethod("append", "(Ljava/lang/String;)Ljava/lang/StringBuffer;");
             addMethod("append", "(I)Ljava/lang/StringBuffer;");
             addMethod("append", "(Ljava/lang/Object;)Ljava/lang/StringBuffer;");
             addMethod("toString", "()Ljava/lang/String;");

             loadedClasses[className] = javaClass;
             return javaClass;
        }
        
        std::cerr << "Class not found: " << className << std::endl;
        return nullptr;
    }

    auto data = jarLoader.getFile(path);
    if (!data) return nullptr;

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
        return javaClass;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse class " << className << ": " << e.what() << std::endl;
        return nullptr;
    }
}

std::optional<JavaValue> Interpreter::execute(std::shared_ptr<StackFrame> frame) {
    if (!frame) return std::nullopt;
    
    // std::cerr << "Entering method: " << frame->methodInfo.name_index << std::endl; 
    // Need proper name resolution to be useful
    
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
        // std::cerr << "No code attribute (native?)" << std::endl;
        return std::nullopt;
    }

    util::DataReader codeReader(code);
    
    while (codeReader.tell() < code.size()) {
        try {
            std::optional<JavaValue> ret;
            if (executeInstruction(frame, codeReader, ret)) {
                return ret;
            }
        } catch (const std::exception& e) {
            std::cerr << "Runtime Exception: " << e.what() << std::endl;
            // Print stack trace or context
            std::cerr << "Method context: " << frame->classFile->this_class << std::endl; // Not very helpful without names
            throw; // Re-throw to propagate
        }
    }
    
    return std::nullopt;
}

bool Interpreter::executeInstruction(std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, std::optional<JavaValue>& returnVal) {
    uint8_t opcode = codeReader.readU1();
    // std::cerr << "Opcode: 0x" << std::hex << (int)opcode << std::dec << " at PC " << (codeReader.tell()-1) << " Stack: " << frame->stackSize() << std::endl;
    
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
                val.val.ref = nullptr; // Ensure no garbage pointer
                frame->push(val);
            } else if (auto integer = std::dynamic_pointer_cast<ConstantInteger>(constant)) {
                JavaValue val; val.type = JavaValue::INT; val.val.i = integer->bytes;
                frame->push(val);
            } else if (auto flt = std::dynamic_pointer_cast<ConstantFloat>(constant)) {
                JavaValue val; val.type = JavaValue::FLOAT; val.val.f = flt->bytes;
                frame->push(val);
            } else {
                 std::cerr << "Unsupported LDC type at index " << (int)index << std::endl;
            }
            break;
        }

        // --- Loads ---
        case OP_ILOAD: { uint8_t idx = codeReader.readU1(); frame->push(frame->getLocal(idx)); break; }
        case OP_ILOAD_0: frame->push(frame->getLocal(0)); break;
        case OP_ILOAD_1: frame->push(frame->getLocal(1)); break;
        case OP_ILOAD_2: frame->push(frame->getLocal(2)); break;
        case OP_ILOAD_3: frame->push(frame->getLocal(3)); break;

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
        
        case OP_ALOAD: { uint8_t idx = codeReader.readU1(); frame->push(frame->getLocal(idx)); break; }
        case OP_ALOAD_0: {
            JavaValue v = frame->getLocal(0);
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
        
        // --- Stack ---
        case OP_POP: frame->pop(); break;
        case OP_DUP: frame->push(frame->peek()); break;
        case OP_DUP_X1: {
            JavaValue v1 = frame->pop();
            JavaValue v2 = frame->pop();
            frame->push(v1);
            frame->push(v2);
            frame->push(v1);
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

        // --- Objects ---
        case OP_NEW: {
            uint16_t index = codeReader.readU2();
            auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[index]);
            auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
            
            auto cls = resolveClass(className->bytes);
            if (!cls) {
                throw std::runtime_error("Could not find class: " + className->bytes);
            }
            
            JavaObject* obj = HeapManager::getInstance().allocate(cls);
            JavaValue val;
            val.type = JavaValue::REFERENCE;
            val.val.ref = obj;
            frame->push(val);
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
            // Look up offset
            auto it = obj->cls->fieldOffsets.find(name->bytes);
            if (it == obj->cls->fieldOffsets.end()) {
                throw std::runtime_error("Field not found: " + name->bytes);
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
            if (!obj->cls) throw std::runtime_error("Object class is null");
            
            // Look up offset
            auto it = obj->cls->fieldOffsets.find(name->bytes);
            if (it == obj->cls->fieldOffsets.end()) {
                throw std::runtime_error("Field not found: " + name->bytes);
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
                 
                 if (className->bytes == "java/lang/System") {
                     JavaValue val;
                     val.type = JavaValue::REFERENCE;
                     val.val.ref = (void*)0xDEADBEEF; 
                     frame->push(val);
                     break;
                 }
                 
                 // Regular GETSTATIC
                 auto nameAndType = std::dynamic_pointer_cast<ConstantNameAndType>(frame->classFile->constant_pool[ref->name_and_type_index]);
                 auto name = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->name_index]);
                 
                 auto cls = resolveClass(className->bytes);
                 if (!cls) throw std::runtime_error("Class not found: " + className->bytes);
                 
                 // Initialize class if needed (skipped for now)
                 
                 auto it = cls->staticFields.find(name->bytes);
                 if (it == cls->staticFields.end()) {
                     // Default value 0/null
                     JavaValue val;
                     val.type = JavaValue::INT; // Or REF, determined by usage context usually, but here we just push 0
                     val.val.l = 0; 
                     frame->push(val);
                 } else {
                     JavaValue val;
                     // We don't know type easily without parsing descriptor, but we stored as int64_t
                     val.val.l = it->second;
                     // Guess type? For now just use LONG as container
                     val.type = JavaValue::LONG; // Consumers will cast
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
                 
                 auto cls = resolveClass(className->bytes);
                 if (!cls) throw std::runtime_error("Class not found: " + className->bytes);
                 
                 JavaValue val = frame->pop();
                 // Store value
                 cls->staticFields[name->bytes] = val.val.l; // Store as 64-bit
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

             bool isStatic = (opcode == OP_INVOKESTATIC);

             // Calculate arg count first
             int argCount = 0;
             for (char c : descriptor->bytes) {
                 if (c == 'L') argCount++; // Ref
                 if (c == 'I') argCount++; // Int
                 // Ignore array [ and ;
             }
             // Hack for Image.createImage(String) -> (Ljava/lang/String;)I -> 1 arg
             if (name->bytes == "createImage" || name->bytes == "createImageNative") argCount = 1;
             
             if (!isStatic) argCount++; // 'this'

             // Special handling for Object/Canvas constructors (skip them)
             if (!isStatic && name->bytes == "<init>") {
                 if (className->bytes == "java/lang/Object" || className->bytes == "javax/microedition/lcdui/Canvas") {
                     // Pop args and return
                     for(int i=0; i<argCount; ++i) frame->pop();
                     break;
                 }
             }

             // Normal method call (Static or Special or Init)
             // Check if native
             auto nativeFunc = NativeRegistry::getInstance().getNative(className->bytes, name->bytes, descriptor->bytes);
             if (nativeFunc) {
                 // Native call!
                 nativeFunc(frame);
             } else {
                 // Java method call
                 auto cls = resolveClass(className->bytes);
                 if (!cls) throw std::runtime_error("Class not found: " + className->bytes);

                 bool found = false;
                 for (const auto& m : cls->rawFile->methods) {
                     auto mName = std::dynamic_pointer_cast<ConstantUtf8>(cls->rawFile->constant_pool[m.name_index]);
                     auto mDesc = std::dynamic_pointer_cast<ConstantUtf8>(cls->rawFile->constant_pool[m.descriptor_index]);
                     if (mName->bytes == name->bytes && mDesc->bytes == descriptor->bytes) {
                         auto newFrame = std::make_shared<StackFrame>(m, cls->rawFile);
                         
                         for (int i = argCount - 1; i >= 0; i--) {
                             newFrame->setLocal(i, frame->pop());
                         }
                         
                         auto ret = execute(newFrame);
                         
                         // Push return value if any
                         if (descriptor->bytes.back() != 'V') {
                             if (ret) {
                                 frame->push(*ret);
                             }
                         }
                         
                         found = true;
                         break;
                     }
                 }
                 if (!found) std::cerr << "Method not found: " << className->bytes << "." << name->bytes << std::endl;
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
                 
                 // Find method
                 bool found = false;
                 // Need to look up method in class hierarchy!
                 // For now just current class.
                 
                 for (const auto& m : cls->rawFile->methods) {
                     auto mName = std::dynamic_pointer_cast<ConstantUtf8>(cls->rawFile->constant_pool[m.name_index]);
                     auto mDesc = std::dynamic_pointer_cast<ConstantUtf8>(cls->rawFile->constant_pool[m.descriptor_index]);
                     if (mName->bytes == name->bytes && mDesc->bytes == descriptor->bytes) {
                         // std::cerr << "Invoke " << cls->name << "." << name->bytes << descriptor->bytes << " Flags: " << std::hex << m.access_flags << std::dec << std::endl;
                         
                         // Check if native
                         // FORCE Native for StringBuffer to avoid mixed Java/Native issues with mock class
                         if ((m.access_flags & 0x0100) || cls->name == "java/lang/StringBuffer") { // ACC_NATIVE
                             // Restore stack for native call
                             frame->push(obj);
                             for (int i = argCount - 1; i >= 0; i--) {
                                 frame->push(args[i]);
                             }
                             
                             auto nativeFunc = NativeRegistry::getInstance().getNative(cls->name, name->bytes, descriptor->bytes);
                             if (nativeFunc) {
                                 nativeFunc(frame);
                             } else {
                                 std::cerr << "UnsatisfiedLinkError (virtual): " << cls->name << "." << name->bytes << descriptor->bytes << std::endl;
                             }
                         } else {
                             auto newFrame = std::make_shared<StackFrame>(m, cls->rawFile);
                             newFrame->setLocal(0, obj); // this
                             
                             // Args are popped in reverse: last arg is at args[0]
                             // setLocal(1) should be first arg.
                             // args[argCount-1] is first arg.
                             for(int i=0; i<argCount; ++i) {
                                 newFrame->setLocal(i+1, args[argCount - 1 - i]);
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
                 if (!found) std::cerr << "Method not found: " << name->bytes << std::endl;
            }
            break;
        }
        
        case OP_RETURN: return false; // Stop execution
        case OP_IRETURN:
        case OP_ARETURN: {
            returnVal = frame->pop();
            return false;
        }
        
        default:
            std::cerr << "Unknown Opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
            break;
    }
    return true; // Continue execution
}

} // namespace core
} // namespace j2me
