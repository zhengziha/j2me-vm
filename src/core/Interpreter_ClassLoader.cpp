#include "Interpreter.hpp"
#include "ClassParser.hpp"
#include "Logger.hpp"
#include <iostream>

namespace j2me {
namespace core {

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

                     // Resolve interfaces
                     for (uint16_t interfaceIndex : rawFile->interfaces) {
                         if (interfaceIndex > 0 && interfaceIndex < rawFile->constant_pool.size()) {
                             auto interfaceInfo = std::dynamic_pointer_cast<ConstantClass>(rawFile->constant_pool[interfaceIndex]);
                             if (interfaceInfo) {
                                 auto interfaceNameInfo = std::dynamic_pointer_cast<ConstantUtf8>(rawFile->constant_pool[interfaceInfo->name_index]);
                                 if (interfaceNameInfo) {
                                     auto interfaceClass = resolveClass(interfaceNameInfo->bytes);
                                     javaClass->interfaces.push_back(interfaceClass);
                                 }
                             }
                         }
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
             addMethod("read", "([BII)I");
             addMethod("available", "()I");
             addMethod("skip", "(J)J");
             addMethod("close", "()V");
             addMethod("mark", "(I)V");
             addMethod("reset", "()V");
             addMethod("markSupported", "()Z");

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
             javaClass->fieldOffsets["value|[B"] = 0;
             javaClass->fieldOffsets["offset|I"] = 1;
             javaClass->fieldOffsets["count|I"] = 2;
             
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

        // Mock j2me/media/DummyPlayer
        if (className == "j2me/media/DummyPlayer") {
             auto dummy = std::make_shared<ClassFile>();
             auto javaClass = std::make_shared<JavaClass>(dummy);
             javaClass->name = "j2me/media/DummyPlayer";
             javaClass->instanceSize = 0;
             
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

             addMethod("start", "()V");
             addMethod("stop", "()V");
             addMethod("close", "()V");
             addMethod("deallocate", "()V");
             addMethod("prefetch", "()V");
             addMethod("realize", "()V");
             addMethod("setLoopCount", "(I)V");
             addMethod("getState", "()I");
             addMethod("getContentType", "()Ljava/lang/String;");
             addMethod("getControl", "(Ljava/lang/String;)Ljavax/microedition/media/Control;");
             addMethod("getControls", "()[Ljavax/microedition/media/Control;");
             addMethod("addPlayerListener", "(Ljavax/microedition/media/PlayerListener;)V");
             addMethod("removePlayerListener", "(Ljavax/microedition/media/PlayerListener;)V");

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
                 javaClass->link(nullptr);
            }
        } else {
            // This is java/lang/Object (or invalid)
            javaClass->link(nullptr);
        }

        // Resolve interfaces
        for (uint16_t interfaceIndex : rawFile->interfaces) {
            if (interfaceIndex > 0 && interfaceIndex < rawFile->constant_pool.size()) {
                auto interfaceInfo = std::dynamic_pointer_cast<ConstantClass>(rawFile->constant_pool[interfaceIndex]);
                if (interfaceInfo) {
                    auto interfaceNameInfo = std::dynamic_pointer_cast<ConstantUtf8>(rawFile->constant_pool[interfaceInfo->name_index]);
                    if (interfaceNameInfo) {
                        auto interfaceClass = resolveClass(interfaceNameInfo->bytes);
                        javaClass->interfaces.push_back(interfaceClass);
                    }
                }
            }
        }

        loadedClasses[className] = javaClass;
        return javaClass;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse class " + className + ": " + e.what());
        throw;
    }
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
    return true;
}

void Interpreter::registerClass(const std::string& className, std::shared_ptr<JavaClass> cls) {
    loadedClasses[className] = cls;
}

}
}
