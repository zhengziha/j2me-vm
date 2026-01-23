#include "javax_microedition_lcdui_Graphics.hpp"
#include "ImageCommon.hpp"
#include "../core/NativeRegistry.hpp"
#include "../platform/GraphicsContext.hpp"
#include "../core/StackFrame.hpp"
#include <iostream>
#include <map>
#include "../core/HeapManager.hpp"
#include <SDL2/SDL.h> // Need SDL types

namespace j2me {
namespace natives {

static int32_t getGraphicsNativePtr(j2me::core::JavaObject* graphicsObj) {
    if (!graphicsObj || !graphicsObj->cls) return 0;
    auto it = graphicsObj->cls->fieldOffsets.find("nativePtr|I");
    if (it == graphicsObj->cls->fieldOffsets.end()) {
        it = graphicsObj->cls->fieldOffsets.find("nativePtr");
    }
    if (it != graphicsObj->cls->fieldOffsets.end()) {
        return (int32_t)graphicsObj->fields[it->second];
    }
    return 0;
}

static SDL_Surface* getTargetSurface(j2me::core::JavaObject* graphicsObj, bool& isScreen) {
    int32_t ptr = getGraphicsNativePtr(graphicsObj);
    if (ptr == 0) {
        isScreen = true;
        return nullptr; // Screen handled by GraphicsContext
    }
    isScreen = false;
    auto it = imageMap.find(ptr);
    if (it != imageMap.end()) {
        return it->second;
    }
    return nullptr;
}

static uint32_t getGraphicsColor(j2me::core::JavaObject* graphicsObj) {
     if (!graphicsObj || !graphicsObj->cls) return 0;
    auto it = graphicsObj->cls->fieldOffsets.find("color|I");
    if (it == graphicsObj->cls->fieldOffsets.end()) {
        it = graphicsObj->cls->fieldOffsets.find("color");
    }
    if (it != graphicsObj->cls->fieldOffsets.end()) {
        return (uint32_t)graphicsObj->fields[it->second];
    }
    return 0;
}

void registerGraphicsNatives(j2me::core::NativeRegistry& registry) {
    // registry passed as argument

    // javax/microedition/lcdui/Graphics.drawImageNative(Ljavax/microedition/lcdui/Image;III)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "drawImageNative", "(Ljavax/microedition/lcdui/Image;III)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int anchor = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            j2me::core::JavaValue imgVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop(); // this
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);
            
            if (imgVal.val.ref != nullptr) {
                // Get nativePtr field
                j2me::core::JavaObject* imgObj = (j2me::core::JavaObject*)imgVal.val.ref;
                
                if (imgObj->fields.size() > 0) {
                     int32_t imgId = (int32_t)imgObj->fields[0];
                     if (imgId != 0) {
                         auto it = imageMap.find(imgId);
                         if (it != imageMap.end()) {
                             SDL_Surface* srcSurface = it->second;
                             
                             if (isScreen) {
                                 /*
                                 static int frameCount = 0;
                                 if (frameCount++ % 60 == 0) { 
                                     std::cout << "[Graphics] drawImage ID: " << imgId << " at " << x << "," << y << std::endl;
                                 }
                                 */
                                 j2me::platform::GraphicsContext::getInstance().drawImage(srcSurface, x, y);
                             } else if (target) {
                                 // Draw to offscreen surface
                                 SDL_Rect dest = {x, y, 0, 0};
                                 SDL_BlitSurface(srcSurface, nullptr, target, &dest);
                             }
                         } else {
                             std::cerr << "[Graphics] drawImage: Invalid Image ID " << imgId << std::endl;
                         }
                     }
                } else {
                    std::cout << "[Graphics] drawImage: Image object has no fields!" << std::endl;
                }
            }
        }
    );

    // javax/microedition/lcdui/Graphics.drawLineNative(IIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "drawLineNative", "(IIII)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int y2 = frame->pop().val.i;
            int x2 = frame->pop().val.i;
            int y1 = frame->pop().val.i;
            int x1 = frame->pop().val.i;
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);

            if (isScreen) {
                j2me::platform::GraphicsContext::getInstance().drawLine(x1, y1, x2, y2);
            } else if (target) {
                // Offscreen line drawing
                // For now, use a simple pixel setter or FillRect for dots
                uint32_t color = getGraphicsColor(graphicsObj);
                // Simple Bresenham? Or just skip for now?
                // Let's implement a simple horizontal/vertical or just dots.
                // SDL_FillRect for 1px?
                SDL_Rect r = {x1, y1, 1, 1};
                SDL_FillRect(target, &r, color); 
                // TODO: proper line drawing
            }
        }
    );

    // javax/microedition/lcdui/Graphics.fillRectNative(IIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "fillRectNative", "(IIII)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int h = frame->pop().val.i;
            int w = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);
            
            if (isScreen) {
                j2me::platform::GraphicsContext::getInstance().fillRect(x, y, w, h);
            } else if (target) {
                uint32_t color = getGraphicsColor(graphicsObj);
                // Convert 0x00RRGGBB to SDL format?
                // Java color is usually 0xAARRGGBB (but Graphics.setColor sets 0x00RRGGBB)
                // We should map it.
                // But wait, setColor in Graphics.java sets 'color' field.
                // We retrieved it raw.
                // SDL_MapRGB expects r, g, b.
                uint8_t r = (color >> 16) & 0xFF;
                uint8_t g = (color >> 8) & 0xFF;
                uint8_t b = (color) & 0xFF;
                uint32_t sdlColor = SDL_MapRGBA(target->format, r, g, b, 255);
                
                SDL_Rect rect = {x, y, w, h};
                SDL_FillRect(target, &rect, sdlColor);
            }
        }
    );

    // javax/microedition/lcdui/Graphics.setColorNative(III)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "setColorNative", "(III)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int b = frame->pop().val.i;
            int g = frame->pop().val.i;
            int r = frame->pop().val.i;
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            
            bool isScreen;
            getTargetSurface(graphicsObj, isScreen);
            
            if (isScreen) {
                j2me::platform::GraphicsContext::getInstance().setColor(r, g, b);
            }
            // For offscreen, the color is stored in Java object 'color' field and used in draw methods.
            // We don't need to do anything here for offscreen as we read the field.
        }
    );
    
    // javax/microedition/lcdui/Graphics.drawStringNative(Ljava/lang/String;III)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "drawStringNative", "(Ljava/lang/String;III)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int anchor = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            j2me::core::JavaValue strVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);
            
            if (strVal.type == j2me::core::JavaValue::REFERENCE && !strVal.strVal.empty()) {
                 if (isScreen) {
                    // Placeholder
                    j2me::platform::GraphicsContext::getInstance().fillRect(x, y, 50, 10);
                 } else if (target) {
                     // Placeholder
                     uint32_t color = getGraphicsColor(graphicsObj);
                     uint8_t r = (color >> 16) & 0xFF;
                     uint8_t g = (color >> 8) & 0xFF;
                     uint8_t b = (color) & 0xFF;
                     uint32_t sdlColor = SDL_MapRGBA(target->format, r, g, b, 255);
                     
                     SDL_Rect rect = {x, y, 50, 10};
                     SDL_FillRect(target, &rect, sdlColor);
                 }
            }
        }
    );

    // javax/microedition/lcdui/Graphics.setFontNative(Ljavax/microedition/lcdui/Font;)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "setFontNative", "(Ljavax/microedition/lcdui/Font;)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // Font parameter
            frame->pop(); // this
            // Mock implementation: do nothing for now, as we don't have font support yet
        }
    );
}

} // namespace natives
} // namespace j2me
