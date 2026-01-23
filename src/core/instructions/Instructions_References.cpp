#include "../Interpreter.hpp"
#include "../Opcodes.hpp"
#include "../HeapManager.hpp"
#include "../ClassFile.hpp"
#include "../NativeRegistry.hpp"
#include "../Logger.hpp"
#include <iostream>
#include <cstring>
#include <algorithm>

namespace j2me {
namespace core {

void Interpreter::initReferences() {
    instructionTable[OP_NEWARRAY] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            uint8_t atype = codeReader.readU1();
            int32_t count = frame->pop().val.i;
            if (count < 0) throw std::runtime_error("NegativeArraySizeException");
            
            JavaObject* obj = new JavaObject(nullptr);
            obj->fields.resize(count);
            
            JavaValue val;
            val.type = JavaValue::REFERENCE;
            val.val.ref = obj;
            frame->push(val);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_ANEWARRAY] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            uint16_t index = codeReader.readU2();
            auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[index]);
            auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
            
            int32_t count = frame->pop().val.i;
            if (count < 0) throw std::runtime_error("NegativeArraySizeException");
            
            JavaObject* obj = new JavaObject(nullptr);
            obj->fields.resize(count);
            
            JavaValue val;
            val.type = JavaValue::REFERENCE;
            val.val.ref = obj;
            frame->push(val);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_ARRAYLENGTH] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            JavaValue arrRef = frame->pop();
            if (arrRef.val.ref == nullptr) throw std::runtime_error("NullPointerException");
            
            JavaObject* arr = (JavaObject*)arrRef.val.ref;
            JavaValue val;
            val.type = JavaValue::INT;
            val.val.i = arr->fields.size();
            frame->push(val);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_NEW] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            uint16_t index = codeReader.readU2();
            auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[index]);
            auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
            
            if (!isValidClassName(className->bytes)) {
                throw std::runtime_error("Invalid class name in OP_NEW: " + className->bytes);
            }
            
            auto cls = resolveClass(className->bytes);
            if (!cls) {
                throw std::runtime_error("Could not find class: " + className->bytes);
            }
            
            if (initializeClass(thread, cls)) {
                codeReader.seek(codeReader.tell() - 3);
                return true;
            }
            
            JavaObject* obj = HeapManager::getInstance().allocate(cls);
            JavaValue val;
            val.type = JavaValue::REFERENCE;
            val.val.ref = obj;
            frame->push(val);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_ATHROW] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            JavaValue exceptionVal = frame->pop();
            if (exceptionVal.val.ref == nullptr) {
                throw std::runtime_error("NullPointerException in ATHROW");
            }
            
            JavaObject* exObj = (JavaObject*)exceptionVal.val.ref;
            std::string exClass = "Unknown";
            if (exObj->cls) exClass = exObj->cls->name;
            
            throw std::runtime_error("Unhandled Java Exception: " + exClass);
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_CHECKCAST] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            uint16_t index = codeReader.readU2();
            auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[index]);
            auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
            
            JavaValue objVal = frame->peek();
            if (objVal.val.ref == nullptr) {
                break;
            }
            
            JavaObject* obj = static_cast<JavaObject*>(objVal.val.ref);
            if (!obj->cls) {
                 if (className->bytes == "java/lang/Object") break;
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
                LOG_ERROR("ClassCastException: " + obj->cls->name + " cannot be cast to " + className->bytes);
                throw std::runtime_error("ClassCastException: " + obj->cls->name + " to " + className->bytes);
            }
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_INSTANCEOF] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
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
        } while(0);
        return true;
    };

    instructionTable[OP_GETFIELD] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            uint16_t index = codeReader.readU2();
            if (index >= frame->classFile->constant_pool.size()) throw std::runtime_error("CP index out of bounds");
            
            auto fieldRef = std::dynamic_pointer_cast<ConstantRef>(frame->classFile->constant_pool[index]);
            if (!fieldRef) throw std::runtime_error("Invalid field ref in GETFIELD");
            
            auto nameAndType = std::dynamic_pointer_cast<ConstantNameAndType>(frame->classFile->constant_pool[fieldRef->name_and_type_index]);
            if (!nameAndType) throw std::runtime_error("Invalid nameAndType in GETFIELD");
            
            auto name = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->name_index]);
            if (!name) throw std::runtime_error("Invalid name in GETFIELD");
            
            auto descriptor = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->descriptor_index]);
            if (!descriptor) throw std::runtime_error("Invalid descriptor in GETFIELD");
            
            JavaValue objVal = frame->pop();
            
            JavaObject* obj = static_cast<JavaObject*>(objVal.val.ref);
            
            if (!obj) throw std::runtime_error("Object is null (checked)");
            if (!obj->cls) {
                 // Check if it's array length
                 if (name->bytes == "length") {
                    JavaValue fieldVal;
                    fieldVal.type = JavaValue::INT;
                    fieldVal.val.i = obj->fields.size();
                    frame->push(fieldVal);
                    break;
                 }
                 throw std::runtime_error("Object class is null");
            }
            
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
        } while(0);
        return true;
    };

    instructionTable[OP_PUTFIELD] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
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
            
            if (obj->cls == nullptr && name->bytes == "length") {
                break;
            }
            
            if (!obj->cls) throw std::runtime_error("Object class is null");
            
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
        } while(0);
        return true;
    };

    instructionTable[OP_GETSTATIC] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            uint16_t index = codeReader.readU2();
            auto ref = std::dynamic_pointer_cast<ConstantRef>(frame->classFile->constant_pool[index]);
            if (ref) {
                 auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[ref->class_index]);
                 auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
                 
                 if (!isValidClassName(className->bytes)) {
                     throw std::runtime_error("Invalid class name in GETSTATIC: " + className->bytes);
                 }
                 
                 auto nameAndType = std::dynamic_pointer_cast<ConstantNameAndType>(frame->classFile->constant_pool[ref->name_and_type_index]);
                 auto name = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->name_index]);
                 
                 auto cls = resolveClass(className->bytes);
                 if (!cls) throw std::runtime_error("Class not found: " + className->bytes);
                 
                 if (initializeClass(thread, cls)) {
                     codeReader.seek(codeReader.tell() - 3);
                     return true;
                 }
                 
                 auto descriptor = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->descriptor_index]);
                 auto it = cls->staticFields.find(name->bytes + "|" + descriptor->bytes);
                 if (it == cls->staticFields.end()) {
                     JavaValue val;
                     if (descriptor && descriptor->bytes[0] == 'L') {
                         val.type = JavaValue::REFERENCE;
                         val.val.ref = nullptr;
                     } else {
                         val.type = JavaValue::INT;
                         val.val.i = 0;
                     }
                     frame->push(val);
                 } else {
                     JavaValue val;
                     if (descriptor && descriptor->bytes[0] == 'L') {
                         val.type = JavaValue::REFERENCE;
                         val.val.ref = (void*)it->second;
                     } else {
                         val.type = JavaValue::LONG;
                         val.val.l = it->second;
                     }
                     frame->push(val);
                 }
                 break;
            }
            std::cerr << "Unsupported GETSTATIC index: " << index << std::endl;
            break;
        } while(0);
        return true;
    };

    instructionTable[OP_PUTSTATIC] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            uint16_t index = codeReader.readU2();
            auto ref = std::dynamic_pointer_cast<ConstantRef>(frame->classFile->constant_pool[index]);
            if (ref) {
                 auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[ref->class_index]);
                 auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
                 auto nameAndType = std::dynamic_pointer_cast<ConstantNameAndType>(frame->classFile->constant_pool[ref->name_and_type_index]);
                 auto name = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->name_index]);
                 auto descriptor = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->descriptor_index]);
                 
                 if (!isValidClassName(className->bytes)) {
                     throw std::runtime_error("Invalid class name in PUTSTATIC: " + className->bytes);
                 }
                 
                 auto cls = resolveClass(className->bytes);
                 if (!cls) throw std::runtime_error("Class not found: " + className->bytes);
                 
                 if (initializeClass(thread, cls)) {
                     codeReader.seek(codeReader.tell() - 3);
                     return true;
                 }
                 
                 JavaValue val = frame->pop();
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
        } while(0);
        return true;
    };

    instructionTable[OP_INVOKESPECIAL] = instructionTable[OP_INVOKESTATIC] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
             uint16_t index = codeReader.readU2();
             auto methodRef = std::dynamic_pointer_cast<ConstantRef>(frame->classFile->constant_pool[index]);
             auto nameAndType = std::dynamic_pointer_cast<ConstantNameAndType>(frame->classFile->constant_pool[methodRef->name_and_type_index]);
             auto name = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->name_index]);
             auto descriptor = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->descriptor_index]);
             
             auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[methodRef->class_index]);
             auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);

             if (!isValidClassName(className->bytes)) {
                 throw std::runtime_error("Invalid class name in INVOKE: " + className->bytes);
             }

             bool isStatic = (opcode == OP_INVOKESTATIC);

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
                         argCount++;
                         i++;
                         while (i < descriptor->bytes.length() && descriptor->bytes[i] != ';') {
                             i++;
                         }
                         i++;
                     } else if (c == '[') {
                         i++;
                     } else {
                         argCount++;
                         i++;
                     }
                 } else {
                     i++;
                 }
             }
             
             if (!isStatic) argCount++; // 'this'

             auto cls = resolveClass(className->bytes);
             if (!cls) throw std::runtime_error("Class not found: " + className->bytes);
             
             if (isStatic) {
                 if (initializeClass(thread, cls)) {
                     codeReader.seek(codeReader.tell() - 3);
                     return true;
                 }
             }

             bool found = false;
             for (const auto& m : cls->rawFile->methods) {
                 auto mName = std::dynamic_pointer_cast<ConstantUtf8>(cls->rawFile->constant_pool[m.name_index]);
                 auto mDesc = std::dynamic_pointer_cast<ConstantUtf8>(cls->rawFile->constant_pool[m.descriptor_index]);
                 
                 if (mName->bytes == name->bytes && mDesc->bytes == descriptor->bytes) {
                     if (m.access_flags & 0x0100) { // ACC_NATIVE
                         auto nativeFunc = NativeRegistry::getInstance().getNative(className->bytes, name->bytes, descriptor->bytes);
                        if (nativeFunc) {
                            nativeFunc(thread, frame);
                        } else {
                            std::cerr << "UnsatisfiedLinkError: " << className->bytes << "." << name->bytes << descriptor->bytes << std::endl;
                             throw std::runtime_error("UnsatisfiedLinkError: " + className->bytes + "." + name->bytes + descriptor->bytes);
                         }
                     } else {
                         if (cls->name == "java/lang/Object" && name->bytes == "<init>") {
                             found = true;
                             break;
                         }
                         
                         auto newFrame = std::make_shared<StackFrame>(m, cls->rawFile);
                         
                         std::vector<JavaValue> args;
                         for (int i = 0; i < argCount; i++) {
                             args.push_back(frame->pop());
                         }
                         
                        int localIndex = 0;
                        for (int i = 0; i < argCount; i++) {
                            JavaValue& val = args[argCount - 1 - i];
                            newFrame->setLocal(localIndex, val);
                            localIndex++;
                            if (val.type == JavaValue::LONG || val.type == JavaValue::DOUBLE) {
                                localIndex++;
                            }
                        }
                        
                        thread->pushFrame(newFrame);
                    }
                    found = true;
                    break;
                }
            }
             
             if (!found) {
                 auto nativeFunc = NativeRegistry::getInstance().getNative(className->bytes, name->bytes, descriptor->bytes);
                if (nativeFunc) {
                     nativeFunc(thread, frame);
                } else {
                     std::cerr << "Method not found: " << className->bytes << "." << name->bytes << std::endl;
                      throw std::runtime_error("Method not found: " + className->bytes + "." + name->bytes);
                 }
             }

             break;
        } while(0);
        return true;
    };

    instructionTable[OP_INVOKEVIRTUAL] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            uint16_t index = codeReader.readU2();
            
            auto methodRef = std::dynamic_pointer_cast<ConstantRef>(frame->classFile->constant_pool[index]);
            auto nameAndType = std::dynamic_pointer_cast<ConstantNameAndType>(frame->classFile->constant_pool[methodRef->name_and_type_index]);
            auto name = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->name_index]);
            auto descriptor = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->descriptor_index]);
            
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
            
            if (!isValidClassName(className->bytes)) {
                throw std::runtime_error("Invalid class name in INVOKEVIRTUAL: " + className->bytes);
            }
            
            std::vector<JavaValue> args;
            for(int i=0; i<argCount; ++i) args.push_back(frame->pop());
            
            JavaValue obj = frame->pop();
            
            if (obj.val.ref == (void*)0xDEADBEEF) {
                if (name->bytes == "println" && !args.empty()) {
                    auto& arg = args[argCount-1]; 
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
                 if (obj.val.ref == nullptr) throw std::runtime_error("NullPointerException");
                 
                 JavaObject* javaObj = static_cast<JavaObject*>(obj.val.ref);
                 auto cls = javaObj->cls;
                 
                 bool found = false;
                 std::shared_ptr<JavaClass> currentClass = cls;

                 while (currentClass != nullptr) {
                     for (const auto& m : currentClass->rawFile->methods) {
                         auto mName = std::dynamic_pointer_cast<ConstantUtf8>(currentClass->rawFile->constant_pool[m.name_index]);
                         auto mDesc = std::dynamic_pointer_cast<ConstantUtf8>(currentClass->rawFile->constant_pool[m.descriptor_index]);
                         
                         if (mName->bytes == name->bytes && mDesc->bytes == descriptor->bytes) {
                             bool isNative = (m.access_flags & 0x0100) != 0;
                             if (currentClass->name == "java/lang/StringBuffer") isNative = true;
                             if (NativeRegistry::getInstance().getNative(currentClass->name, name->bytes, descriptor->bytes)) isNative = true;

                             if (isNative) {
                                 frame->push(obj);
                                 for (int i = argCount - 1; i >= 0; i--) {
                                     frame->push(args[i]);
                                 }
                                 
                                 auto nativeFunc = NativeRegistry::getInstance().getNative(currentClass->name, name->bytes, descriptor->bytes);
                                 if (nativeFunc) {
                                    nativeFunc(thread, frame);
                                 } else {
                                     std::cerr << "UnsatisfiedLinkError (virtual): " << currentClass->name << "." << name->bytes << descriptor->bytes << std::endl;
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
                                 
                                 thread->pushFrame(newFrame);
                            }
                            
                            found = true;
                            break;
                        }
                    }
                     
                     if (found) break;
                     
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
        } while(0);
        return true;
    };

    instructionTable[OP_INVOKEINTERFACE] = [this](std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, uint8_t opcode) -> bool {
        do {
            uint16_t index = codeReader.readU2();
            codeReader.readU1(); // count
            codeReader.readU1(); // 0
            
            auto methodRef = std::dynamic_pointer_cast<ConstantRef>(frame->classFile->constant_pool[index]);
            auto nameAndType = std::dynamic_pointer_cast<ConstantNameAndType>(frame->classFile->constant_pool[methodRef->name_and_type_index]);
            auto name = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->name_index]);
            auto descriptor = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[nameAndType->descriptor_index]);
            
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

            std::vector<JavaValue> args;
            for(int i=0; i<argCount; ++i) args.push_back(frame->pop());
            
            JavaValue obj = frame->pop();
            if (obj.val.ref == nullptr) {
                // Determine Interface Name for logging
                std::string interfaceName = "Unknown";
                if (methodRef) {
                    auto classRef = std::dynamic_pointer_cast<ConstantClass>(frame->classFile->constant_pool[methodRef->class_index]);
                    if (classRef) {
                         auto className = std::dynamic_pointer_cast<ConstantUtf8>(frame->classFile->constant_pool[classRef->name_index]);
                         if (className) interfaceName = className->bytes;
                    }
                }
                
                std::cerr << "[ERROR] NullPointerException in INVOKEINTERFACE: " 
                          << interfaceName << "." << name->bytes << descriptor->bytes << std::endl;
                          
                throw std::runtime_error("NullPointerException");
            }
            
            JavaObject* javaObj = static_cast<JavaObject*>(obj.val.ref);
            auto cls = javaObj->cls;
            
             bool found = false;
             std::shared_ptr<JavaClass> currentClass = cls;

             while (currentClass != nullptr) {
                 for (const auto& m : currentClass->rawFile->methods) {
                     auto mName = std::dynamic_pointer_cast<ConstantUtf8>(currentClass->rawFile->constant_pool[m.name_index]);
                     auto mDesc = std::dynamic_pointer_cast<ConstantUtf8>(currentClass->rawFile->constant_pool[m.descriptor_index]);
                     
                     if (mName->bytes == name->bytes && mDesc->bytes == descriptor->bytes) {
                        if (m.access_flags & 0x0100) { // ACC_NATIVE
                             frame->push(obj);
                             for (int i = argCount - 1; i >= 0; i--) {
                                 frame->push(args[i]);
                             }
                             
                             auto nativeFunc = NativeRegistry::getInstance().getNative(currentClass->name, name->bytes, descriptor->bytes);
                             if (nativeFunc) {
                                 nativeFunc(thread, frame);
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
                            
                            thread->pushFrame(newFrame);
                        }
                         found = true;
                         break;
                     }
                 }
                 if (found) break;
                 
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
        } while(0);
        return true;
    };
}

}
}
