#include "javax_microedition_lcdui_Font.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include <iostream>

namespace j2me {
namespace natives {

void registerFontNatives(j2me::core::NativeRegistry& registry) {
    // javax/microedition/lcdui/Font.getHeight()I
    registry.registerNative("javax/microedition/lcdui/Font", "getHeight", "()I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            // Basic implementation: return a fixed height based on font size
            // Real implementation should check thisVal's fields (face, style, size)
            int height = 16; // Default medium font height
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* fontObj = (j2me::core::JavaObject*)thisVal.val.ref;
                // Assuming fields: face, style, size
                // But we don't know the field layout exactly without the class file.
                // However, standard MIDP Font usually has these.
                // For now, just return a constant.
            }
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = height;
            frame->push(result);
        }
    );

    // javax/microedition/lcdui/Font.stringWidth(Ljava/lang/String;)I
    registry.registerNative("javax/microedition/lcdui/Font", "stringWidth", "(Ljava/lang/String;)I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue strVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            int width = 0;
            if (strVal.type == j2me::core::JavaValue::REFERENCE && !strVal.strVal.empty()) {
                // Approximate width: 8 pixels per char
                width = strVal.strVal.length() * 8;
            } else if (strVal.type == j2me::core::JavaValue::REFERENCE && strVal.val.ref != nullptr) {
                 // It's a string object, but strVal is not populated?
                 // Need to read string object.
                 // But JavaValue for String usually has strVal populated by Interpreter if loaded from constant pool?
                 // Or if it's a runtime object, we need to read it.
                 // For now, assume 0.
            }
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = width;
            frame->push(result);
        }
    );
    
    // javax/microedition/lcdui/Font.getFont(III)Ljavax/microedition/lcdui/Font;
    registry.registerNative("javax/microedition/lcdui/Font", "getFont", "(III)Ljavax/microedition/lcdui/Font;", 
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int size = frame->pop().val.i;
            int style = frame->pop().val.i;
            int face = frame->pop().val.i;
            
            // We need to create a Font object.
            // But we don't have easy access to create object of specific class here without Interpreter context.
            // Actually Interpreter is available via registry if we set it.
            // But NativeRegistry doesn't expose allocate.
            
            // Hack: return null for now, or throw exception.
            // If we return null, app might crash.
            // We need to allocate a Font object.
            
            // Let's rely on Java stub to handle factory if possible.
            // But if getFont is native, we must return an object.
            
            // For now, just print error and return null.
            std::cerr << "Native Font.getFont not fully implemented (cannot allocate)" << std::endl;
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            frame->push(result);
        }
    );
}

} // namespace natives
} // namespace j2me
