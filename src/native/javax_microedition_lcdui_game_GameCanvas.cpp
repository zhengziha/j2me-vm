#include "javax_microedition_lcdui_game_GameCanvas.hpp"
#include "javax_microedition_lcdui_Graphics.hpp"
#include "ImageCommon.hpp"
#include "../core/NativeRegistry.hpp"
#include "../platform/GraphicsContext.hpp"
#include "../core/EventLoop.hpp"
#include "../core/StackFrame.hpp"
#include <iostream>
#include <SDL2/SDL.h>

namespace j2me {
namespace natives {

void registerGameCanvasNatives(j2me::core::NativeRegistry& registry) {
    // javax/microedition/lcdui/game/GameCanvas.getKeyStatesNative()I
    registry.registerNative("javax/microedition/lcdui/game/GameCanvas", "getKeyStatesNative", "()I", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
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
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int h = frame->pop().val.i;
            int w = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            j2me::core::JavaValue imgVal = frame->pop();
            frame->pop(); // this
            
            if (imgVal.val.ref != nullptr) {
                j2me::core::JavaObject* imgObj = (j2me::core::JavaObject*)imgVal.val.ref;
                if (imgObj->fields.size() > 0) {
                     int32_t imgId = (int32_t)imgObj->fields[0];
                     auto it = imageMap.find(imgId);
                     if (it != imageMap.end()) {
                         SDL_Surface* srcSurface = it->second;
                         
                         // Draw srcSurface to Screen
                         // Note: x, y, w, h are region on SCREEN to update?
                         // Or source region?
                         // MIDP docs: flushGraphics(x, y, width, height) flushes the specified region of the offscreen buffer to the display.
                         // It means: copy rect (x,y,w,h) from offscreen to (x,y) on screen.
                         
                         // We assume offscreen buffer is same size as screen (or handled by GraphicsContext).
                         // GraphicsContext::drawImage usually draws the whole image at (x,y).
                         // But here we want to blit a region.
                         
                         // Since our GraphicsContext::drawImage is simple (whole image), let's use it for now if x=0, y=0.
                         // But for partial updates, we might need a better API.
                         // However, SDL_BlitSurface supports source and dest rects.
                         // GraphicsContext doesn't expose BlitSurface directly.
                         // But we can add a method or just rely on drawImage for now.
                         
                         // Let's modify GraphicsContext to support drawing parts or just draw the whole thing if typically used for double buffering.
                         // Usually GameCanvas is full screen.
                         
                         // Debug log
                         static int frameCount = 0;
                         if (frameCount++ % 60 == 0) {
                             std::cout << "[GameCanvas] flushGraphicsNative called" << std::endl;
                         }
                         
                         j2me::platform::GraphicsContext::getInstance().drawImage(srcSurface, 0, 0);
                     }
                }
            }
        }
    );
}

} // namespace natives
} // namespace j2me