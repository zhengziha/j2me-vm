#include "javax_microedition_lcdui_Display.hpp"
#include "../core/NativeRegistry.hpp"
#include <iostream>

namespace j2me {
namespace natives {

static j2me::core::JavaObject* g_currentDisplayable = nullptr;

j2me::core::JavaObject* getCurrentDisplayable() {
    return g_currentDisplayable;
}

void registerDisplayNatives(j2me::core::NativeRegistry& registry) {
    // registry passed as argument

    // javax/microedition/lcdui/Display.getDisplay(Ljavax/microedition/midlet/MIDlet;)Ljavax/microedition/lcdui/Display;

    // javax/microedition/lcdui/Canvas.setFullScreenMode(Z)V
    registry.registerNative("javax/microedition/lcdui/Canvas", "setFullScreenMode", "(Z)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            bool mode = frame->pop().val.i != 0;
            frame->pop(); // this
            std::cout << "[Canvas] setFullScreenMode: " << (mode ? "true" : "false") << std::endl;
        }
    );
    
    // javax/microedition/lcdui/Displayable.getWidth()I
    registry.registerNative("javax/microedition/lcdui/Displayable", "getWidth", "()I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // this
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = 240; // Default width
            frame->push(ret);
        }
    );

    // javax/microedition/lcdui/Displayable.getHeight()I
    registry.registerNative("javax/microedition/lcdui/Displayable", "getHeight", "()I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // this
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = 320; // Default height
            frame->push(ret);
        }
    );
    
    // Also register on Canvas just in case
    registry.registerNative("javax/microedition/lcdui/Canvas", "getWidth", "()I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // this
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = 240; 
            frame->push(ret);
        }
    );
    registry.registerNative("javax/microedition/lcdui/Canvas", "getHeight", "()I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // this
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = 320;
            frame->push(ret);
        }
    );

    // javax/microedition/lcdui/Display.setCurrentNative(Ljavax/microedition/lcdui/Displayable;)V
    registry.registerNative("javax/microedition/lcdui/Display", "setCurrentNative", "(Ljavax/microedition/lcdui/Displayable;)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue displayableVal = frame->pop();
            frame->pop(); // this (Display instance)
            
            g_currentDisplayable = (j2me::core::JavaObject*)displayableVal.val.ref;
            std::cout << "[Display] Set current displayable: " << g_currentDisplayable << std::endl;
            if (g_currentDisplayable && g_currentDisplayable->cls) {
                 std::cout << "[Display] Class: " << g_currentDisplayable->cls->name << std::endl;
            }
        }
    );
}

}
}
