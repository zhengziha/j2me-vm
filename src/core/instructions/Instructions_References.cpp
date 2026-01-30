#include "../Interpreter.hpp"
#include "../RuntimeTypes.hpp"
#include "../Opcodes.hpp"
#include "../HeapManager.hpp"
#include "../ClassFile.hpp"
#include "../NativeRegistry.hpp"
#include "../Logger.hpp"
#include "../../native/java_lang_String.hpp"
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
                // Throw NullPointerException
                // TODO: Create NPE object properly
                // For now, print error and throw runtime error to stop VM
                std::cerr << "Error: ATHROW with null exception object (NullPointerException)" << std::endl;
                throw std::runtime_error("NullPointerException in ATHROW");
            }
            
            JavaObject* exObj = (JavaObject*)exceptionVal.val.ref;
            
            // Log exception details before handling
            std::string exClass = "Unknown";
            if (exObj->cls) {
                exClass = exObj->cls->name;
                std::string fieldKey = "detailMessage|Ljava/lang/String;";
                if (exObj->cls->fieldOffsets.find(fieldKey) != exObj->cls->fieldOffsets.end()) {
                     size_t offset = exObj->cls->fieldOffsets[fieldKey];
                     if (offset < exObj->fields.size()) {
                         int64_t msgRef = exObj->fields[offset];
                         if (msgRef != 0) {
                             JavaObject* msgObj = (JavaObject*)msgRef;
                             std::string msg = j2me::natives::getJavaString(msgObj);
                             exClass += " (" + msg + ")";
                         }
                     }
                }
            }
            // LOG_DEBUG("ATHROW: " + exClass);

            if (handleException(thread, exObj)) {
                // Exception handled, PC updated in handleException
                // We need to update codeReader to new PC
                // But codeReader is local to executeInstruction.
                // execute() loop updates frame->pc from codeReader.tell().
                // So we need to sync codeReader to frame->pc.
                
                // However, handleException might have popped frames!
                // So 'frame' variable might be invalid or stale if we popped it.
                // 'thread->currentFrame()' is the new frame.
                
                auto newFrame = thread->currentFrame();
                if (newFrame != frame) {
                    // Frame changed (popped), we return true to continue loop.
                    // But wait, execute() loop uses 'frame' variable which is the old frame?
                    // No, execute() loop gets currentFrame() at start of loop.
                    // But executeInstruction takes 'frame' as argument.
                    // And execute() updates 'frame->pc = codeReader.tell()'.
                    
                    // If we popped the frame, 'frame' is now removed from stack.
                    // Updating 'frame->pc' is pointless but harmless (shared_ptr keeps it alive).
                    // The main loop will start next iteration, get new currentFrame(), and continue.
                    // So we just need to return true.
                    return true; 
                } else {
                    // Same frame (handler found in same method)
                    // We need to update codeReader to jump to handler
                    codeReader.seek(frame->pc);
                    return true;
                }
            } else {
                // Unhandled exception
                throw std::runtime_error("Unhandled Java Exception: " + exClass);
            }
            
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
            
            std::function<bool(std::shared_ptr<JavaClass>, const std::string&)> isAssignable = 
                [&](std::shared_ptr<JavaClass> sub, const std::string& target) -> bool {
                    if (!sub) return false;
                    if (sub->name == target) return true;
                    if (isAssignable(sub->superClass, target)) return true;
                    for (auto& iface : sub->interfaces) {
                         if (isAssignable(iface, target)) return true;
                    }
                    return false;
                };

            if (isAssignable(obj->cls, className->bytes)) {
                found = true;
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
                 std::function<bool(std::shared_ptr<JavaClass>, const std::string&)> isAssignable = 
                     [&](std::shared_ptr<JavaClass> sub, const std::string& target) -> bool {
                         if (!sub) return false;
                         if (sub->name == target) return true;
                         if (isAssignable(sub->superClass, target)) return true;
                         for (auto& iface : sub->interfaces) {
                              if (isAssignable(iface, target)) return true;
                         }
                         return false;
                     };

                 if (isAssignable(obj->cls, className->bytes)) {
                     isInstance = true;
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
                 
                 char typeChar = descriptor ? descriptor->bytes[0] : 'I';
                 
                 if (it == cls->staticFields.end()) {
                     JavaValue val;
                     if (typeChar == 'L' || typeChar == '[') {
                         val.type = JavaValue::REFERENCE;
                         val.val.ref = nullptr;
                     } else if (typeChar == 'J') {
                         val.type = JavaValue::LONG;
                         val.val.l = 0;
                     } else if (typeChar == 'D') {
                         val.type = JavaValue::DOUBLE;
                         val.val.d = 0.0;
                     } else if (typeChar == 'F') {
                         val.type = JavaValue::FLOAT;
                         val.val.f = 0.0f;
                     } else {
                         val.type = JavaValue::INT;
                         val.val.i = 0;
                     }
                     frame->push(val);
                 } else {
                     JavaValue val;
                     if (typeChar == 'L' || typeChar == '[') {
                         val.type = JavaValue::REFERENCE;
                         val.val.ref = (void*)it->second;
                     } else if (typeChar == 'J') {
                         val.type = JavaValue::LONG;
                         val.val.l = it->second;
                     } else if (typeChar == 'D') {
                         val.type = JavaValue::DOUBLE;
                         int64_t bits = it->second;
                         memcpy(&val.val.d, &bits, sizeof(double));
                     } else if (typeChar == 'F') {
                         val.type = JavaValue::FLOAT;
                         int32_t bits = (int32_t)it->second;
                         memcpy(&val.val.f, &bits, sizeof(float));
                     } else {
                         val.type = JavaValue::INT;
                         val.val.i = (int32_t)it->second;
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
                     char typeChar = descriptor->bytes[0];
                     if (typeChar == 'J') {
                         cls->staticFields[key] = val.val.l;
                     } else if (typeChar == 'D') {
                         int64_t bits;
                         memcpy(&bits, &val.val.d, sizeof(double));
                         cls->staticFields[key] = bits;
                     } else if (typeChar == 'F') {
                         int32_t bits;
                         memcpy(&bits, &val.val.f, sizeof(float));
                         cls->staticFields[key] = (int64_t)bits;
                     } else {
                         // Int, Short, Byte, Char, Boolean
                         cls->staticFields[key] = (int64_t)val.val.i;
                     }
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
             
             // INTERCEPT: Force native implementation for String(byte[]) to handle encoding (GBK) correctly
              // This bypasses the Java implementation in rt.jar which might default to ISO-8859-1
              if (className->bytes == "java/lang/String" && name->bytes == "<init>") {
                  // std::cout << "DEBUG: String constructor called: " << descriptor->bytes << std::endl;
                  if (descriptor->bytes == "([B)V") {
                      auto nativeFunc = NativeRegistry::getInstance().getNative("java/lang/String", "<init>", "([B)V");
                      if (nativeFunc) {
                          LOG_DEBUG("Intercepted String(byte[])");
                          nativeFunc(thread, frame);
                          break; 
                      }
                  }
                  // Intercept String(byte[], int, int)
                  else if (descriptor->bytes == "([BII)V") {
                      auto nativeFunc = NativeRegistry::getInstance().getNative("java/lang/String", "<init>", "([BII)V");
                      if (nativeFunc) {
                          LOG_DEBUG("Intercepted String(byte[], int, int)");
                          nativeFunc(thread, frame);
                          break;
                      }
                  }
                  // Intercept String(byte[], String)
                  else if (descriptor->bytes == "([BLjava/lang/String;)V") {
                      auto nativeFunc = NativeRegistry::getInstance().getNative("java/lang/String", "<init>", "([BLjava/lang/String;)V");
                      if (nativeFunc) {
                          LOG_DEBUG("Intercepted String(byte[], String)");
                          nativeFunc(thread, frame);
                          break;
                      }
                  }
                  // Intercept String(char[])
                  else if (descriptor->bytes == "([C)V") {
                      auto nativeFunc = NativeRegistry::getInstance().getNative("java/lang/String", "<init>", "([C)V");
                      if (nativeFunc) {
                          // LOG_DEBUG("Intercepted String(char[])");
                          nativeFunc(thread, frame);
                          break;
                      }
                  }
                  // Intercept String(char[], int, int)
                  else if (descriptor->bytes == "([CII)V") {
                      auto nativeFunc = NativeRegistry::getInstance().getNative("java/lang/String", "<init>", "([CII)V");
                      if (nativeFunc) {
                          // LOG_DEBUG("Intercepted String(char[], int, int)");
                          nativeFunc(thread, frame);
                          break;
                      }
                  }
              }
             
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
                         // Object的构造方法只是简单返回，不需要特殊处理
                         // 让正常的构造方法处理逻辑执行
                         
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
                 
                 // Try to use method cache
                 // 尝试使用方法缓存
                 MethodKey cacheKey{className->bytes, name->bytes, descriptor->bytes};
                 auto cacheIt = methodCache.find(cacheKey);
                 
                 std::shared_ptr<JavaClass> methodClass;
                 std::shared_ptr<MethodInfo> method;
                 bool isNative = false;
                 
                 if (cacheIt != methodCache.end()) {
                     methodClass = cacheIt->second.cls;
                     method = cacheIt->second.method;
                     isNative = cacheIt->second.isNative;
                     
                     // Verify the method exists in the class hierarchy
                     // 验证方法在类层次结构中是否存在
                     std::shared_ptr<JavaClass> currentClass = cls;
                     while (currentClass != nullptr) {
                         if (currentClass->name == methodClass->name) {
                             found = true;
                             break;
                         }
                         if (currentClass->rawFile->super_class == 0) {
                             currentClass = nullptr;
                         } else {
                             auto superClassRef = std::dynamic_pointer_cast<ConstantClass>(currentClass->rawFile->constant_pool[currentClass->rawFile->super_class]);
                             auto superClassName = std::dynamic_pointer_cast<ConstantUtf8>(currentClass->rawFile->constant_pool[superClassRef->name_index]);
                             currentClass = resolveClass(superClassName->bytes);
                         }
                     }
                 }
                 
                 if (!found) {
                     std::shared_ptr<JavaClass> currentClass = cls;

                     while (currentClass != nullptr) {
                         for (const auto& m : currentClass->rawFile->methods) {
                             auto mName = std::dynamic_pointer_cast<ConstantUtf8>(currentClass->rawFile->constant_pool[m.name_index]);
                             auto mDesc = std::dynamic_pointer_cast<ConstantUtf8>(currentClass->rawFile->constant_pool[m.descriptor_index]);
                             
                             if (mName->bytes == name->bytes && mDesc->bytes == descriptor->bytes) {
                                 methodClass = currentClass;
                                 method = std::make_shared<MethodInfo>(m);
                                 isNative = (m.access_flags & 0x0100) != 0;
                                 
                                 // Cache the method for future calls
                                 // 缓存方法以供将来调用
                                 methodCache[cacheKey] = {methodClass, method, isNative};
                                 
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
                 }
                 
                 if (!found) {
                    std::string msg = "Method not found: " + cls->name + "." + name->bytes + descriptor->bytes;
                    throw std::runtime_error(msg);
                }
                
                // Execute the method
                // 执行方法
                if (isNative) {
                    frame->push(obj);
                    for (int i = argCount - 1; i >= 0; i--) {
                        frame->push(args[i]);
                    }
                    
                    auto nativeFunc = NativeRegistry::getInstance().getNative(methodClass->name, name->bytes, descriptor->bytes);
                    if (nativeFunc) {
                       nativeFunc(thread, frame);
                    } else {
                        std::cerr << "UnsatisfiedLinkError (virtual): " << methodClass->name << "." << name->bytes << descriptor->bytes << std::endl;
                    }
                } else {
                    auto newFrame = std::make_shared<StackFrame>(*method, methodClass->rawFile);
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
