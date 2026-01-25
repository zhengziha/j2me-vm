#include "javax_microedition_lcdui_Font.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include <iostream>
#include "../platform/GraphicsContext.hpp"
#include "java_lang_String.hpp"

namespace j2me {
namespace natives {

void registerFontNatives(j2me::core::NativeRegistry& registry) {
    // javax/microedition/lcdui/Font.getHeight()I
    registry.registerNative("javax/microedition/lcdui/Font", "getHeight", "()I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            int height = j2me::platform::GraphicsContext::getInstance().getFontHeight();
            if (height <= 0) height = 14;
            
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
            std::string text;
            if (strVal.type == j2me::core::JavaValue::REFERENCE) {
                if (!strVal.strVal.empty()) {
                    text = strVal.strVal;
                } else if (strVal.val.ref != nullptr) {
                    auto strObj = static_cast<j2me::core::JavaObject*>(strVal.val.ref);
                    text = j2me::natives::getJavaString(strObj);
                }
            }
            width = j2me::platform::GraphicsContext::getInstance().measureTextWidth(text);
            
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
