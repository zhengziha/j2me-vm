#include "javax_microedition_lcdui_game_GameCanvas.hpp"
#include "javax_microedition_lcdui_Graphics.hpp"
#include "ImageCommon.hpp"
#include "../core/NativeRegistry.hpp"
#include "../platform/GraphicsContext.hpp"
#include "../core/EventLoop.hpp"
#include "../core/StackFrame.hpp"
#include "../core/Diagnostics.hpp"
#include "../core/Logger.hpp"
#include <iostream>
#include <SDL2/SDL.h>

namespace j2me {
namespace natives {

void registerGameCanvasNatives(j2me::core::NativeRegistry& registry) {
    // javax/microedition/lcdui/game/GameCanvas.getKeyStatesNative()I
    registry.registerNative("javax/microedition/lcdui/game/GameCanvas", "getKeyStatesNative", "()I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // this
            
            int states = j2me::core::EventLoop::getInstance().getKeyStates();
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = states;
            frame->push(ret);
        }
    );

    // javax/microedition/lcdui/game/GameCanvas.flushGraphicsNative(Ljavax/microedition/lcdui/Image;IIII)V
    registry.registerNative("javax/microedition/lcdui/game/GameCanvas", "flushGraphicsNative", "(Ljavax/microedition/lcdui/Image;IIII)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int h = frame->pop().val.i;
            int w = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            j2me::core::JavaValue imgVal = frame->pop();
            frame->pop(); // this

            j2me::core::Diagnostics::getInstance().onGameCanvasFlush();

            if (imgVal.val.ref != nullptr) {
                j2me::core::JavaObject* imgObj = (j2me::core::JavaObject*)imgVal.val.ref;
                if (imgObj->fields.size() > 0) {
                     int32_t imgId = (int32_t)imgObj->fields[0];
                     auto it = imageMap.find(imgId);
                     if (it != imageMap.end()) {
                         SDL_Surface* srcSurface = it->second;

                         // Use drawRegion to support partial flush
                         // flushGraphics(x, y, w, h) copies region (x,y,w,h) from buffer to screen at (x,y)
                         j2me::platform::GraphicsContext::getInstance().drawRegion(srcSurface, x, y, w, h, 0, x, y, 20); // TOP|LEFT = 20
                         j2me::platform::GraphicsContext::getInstance().commit();
                         j2me::platform::GraphicsContext::getInstance().update();

                     }
                }
            }
        }
    );
}

} // namespace natives
} // namespace j2me
