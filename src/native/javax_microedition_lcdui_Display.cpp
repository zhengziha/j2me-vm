#include "javax_microedition_lcdui_Display.hpp"
#include "../core/NativeRegistry.hpp"
#include <iostream>

namespace j2me {
namespace natives {

static j2me::core::JavaObject* g_currentDisplayable = nullptr;

j2me::core::JavaObject* getCurrentDisplayable() {
    return g_currentDisplayable;
}

void registerDisplayNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();

    // javax/microedition/lcdui/Display.setCurrentNative(Ljavax/microedition/lcdui/Displayable;)V
    registry.registerNative("javax/microedition/lcdui/Display", "setCurrentNative", "(Ljavax/microedition/lcdui/Displayable;)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
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
