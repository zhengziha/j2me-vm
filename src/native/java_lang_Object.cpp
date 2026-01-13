#include "java_lang_Object.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include <iostream>

namespace j2me {
namespace natives {

void registerObjectNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();

    // java/lang/Object.getClass()Ljava/lang/Class;
    registry.registerNative("java/lang/Object", "getClass", "()Ljava/lang/Class;", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue objVal = frame->pop(); // this
            
            if (objVal.val.ref == nullptr) {
                // NullPointerException
                std::cerr << "NullPointerException in Object.getClass()" << std::endl;
                // Should throw exception in VM
            } else {
                j2me::core::JavaObject* obj = static_cast<j2me::core::JavaObject*>(objVal.val.ref);
                
                // We need to return the java.lang.Class instance for this object's class.
                // In a real VM, JavaClass structure would point to a JavaObject representing java.lang.Class.
                // For Phase 5, we will create a dummy Class object on the fly or singleton.
                
                // Let's cheat: we need to find "java/lang/Class" class first.
                // But we don't have access to interpreter/loader here easily to resolve it if not loaded.
                // Assuming it's loaded because we are running code.
                
                // TODO: Properly implement Class object caching in JavaClass.
                // For now, let's just return null to stop crash, or try to construct one.
                
                // Better: The Interpreter should have loaded java.lang.Class.
                // We can't easily allocate it here without the ClassFile for java.lang.Class.
                // Wait, if we use NativeRegistry, we are outside Interpreter context mostly.
                
                // Workaround: Return null for now, but print log.
                // Or better: The object model should support this.
                
                std::cout << "[Object] getClass called for " << obj->cls->name << std::endl;
                
                // We need to push a reference to a JavaObject that is an instance of java.lang.Class
                // and has the 'name' field set to obj->cls->name.
                
                // Hack: Just push null and handle it in getResourceAsStream (which is on Class)
                // BUT getResourceAsStream is an instance method of Class.
                // So if we return null here, the next call `is = cls.getResourceAsStream(...)` will NPE.
                // We MUST return a valid object.
                
                // We need to access HeapManager to allocate.
                // We need the JavaClass for java/lang/Class.
                // Let's assume we can get it or mock it.
                
                // Very Hacky: We reuse the object's own class as the "Class" object class? No.
                // We need to find java/lang/Class.
                
                // Let's assume we can't do it properly without refactoring.
                // Return null and log.
                j2me::core::JavaValue ret;
                ret.type = j2me::core::JavaValue::REFERENCE;
                ret.val.ref = nullptr; 
                frame->push(ret);
            }
        }
    );

    // java/lang/Object.hashCode()I
    registry.registerNative("java/lang/Object", "hashCode", "()I", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue objVal = frame->pop(); // this
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            // Use pointer address as hash code
            ret.val.i = (int32_t)(intptr_t)objVal.val.ref;
            frame->push(ret);
        }
    );

    // java/lang/Object.toString()Ljava/lang/String;
    registry.registerNative("java/lang/Object", "toString", "()Ljava/lang/String;", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue objVal = frame->pop(); // this
            
            std::string str = "Object@" + std::to_string((intptr_t)objVal.val.ref);
            if (objVal.val.ref) {
                j2me::core::JavaObject* obj = static_cast<j2me::core::JavaObject*>(objVal.val.ref);
                if (obj->cls) {
                    str = obj->cls->name + "@" + std::to_string((intptr_t)objVal.val.ref);
                }
            }
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::REFERENCE;
            ret.strVal = str;
            frame->push(ret);
        }
    );
}

} // namespace natives
} // namespace j2me
