#include "javax_microedition_lcdui_Graphics.hpp"
#include "java_lang_String.hpp"
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
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
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
                             
                             if (!srcSurface) {
                                 std::cerr << "[Graphics] drawImage: Source surface is NULL for ID " << imgId << std::endl;
                             } else if (isScreen) {
                                 // Draw to screen
                                 j2me::platform::GraphicsContext::getInstance().drawImage(srcSurface, x, y, anchor);
                             } else if (target) {
                                 // Draw to offscreen surface
                                 j2me::platform::GraphicsContext::getInstance().drawImage(srcSurface, x, y, anchor, target);
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
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
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
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
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
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
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
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int anchor = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            j2me::core::JavaValue strVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);
            
            std::string text;
            if (strVal.type == j2me::core::JavaValue::REFERENCE) {
                if (strVal.val.ref != nullptr) {
                    j2me::core::JavaObject* strObj = (j2me::core::JavaObject*)strVal.val.ref;
                    text = j2me::natives::getJavaString(strObj);
                } else if (!strVal.strVal.empty()) {
                    text = strVal.strVal;
                }
            }

            if (!text.empty()) {
                 if (isScreen) {
                    j2me::platform::GraphicsContext::getInstance().drawString(text, x, y, anchor);
                 } else if (target) {
                     // For offscreen, we need the current color
                     uint32_t color = getGraphicsColor(graphicsObj);
                     j2me::platform::GraphicsContext::getInstance().drawString(text, x, y, anchor, color, target);
                 }
            }
        }
    );

    // javax/microedition/lcdui/Graphics.setFontNative(Ljavax/microedition/lcdui/Font;)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "setFontNative", "(Ljavax/microedition/lcdui/Font;)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // Font parameter
            frame->pop(); // this
            // Mock implementation: do nothing for now, as we don't have font support yet
        }
    );

    // javax/microedition/lcdui/Graphics.drawRectNative(IIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "drawRectNative", "(IIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int h = frame->pop().val.i;
            int w = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);
            
            if (isScreen) {
                auto& gc = j2me::platform::GraphicsContext::getInstance();
                gc.drawLine(x, y, x + w - 1, y);
                gc.drawLine(x, y + h - 1, x + w - 1, y + h - 1);
                gc.drawLine(x, y, x, y + h - 1);
                gc.drawLine(x + w - 1, y, x + w - 1, y + h - 1);
            } else if (target) {
                uint32_t color = getGraphicsColor(graphicsObj);
                uint8_t r = (color >> 16) & 0xFF;
                uint8_t g = (color >> 8) & 0xFF;
                uint8_t b = (color) & 0xFF;
                uint32_t sdlColor = SDL_MapRGBA(target->format, r, g, b, 255);
                
                // Top
                SDL_Rect r1 = {x, y, w, 1};
                SDL_FillRect(target, &r1, sdlColor);
                // Bottom
                SDL_Rect r2 = {x, y + h - 1, w, 1};
                SDL_FillRect(target, &r2, sdlColor);
                // Left
                SDL_Rect r3 = {x, y, 1, h};
                SDL_FillRect(target, &r3, sdlColor);
                // Right
                SDL_Rect r4 = {x + w - 1, y, 1, h};
                SDL_FillRect(target, &r4, sdlColor);
            }
        }
    );

    // javax/microedition/lcdui/Graphics.setClipNative(IIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "setClipNative", "(IIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int h = frame->pop().val.i;
            int w = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);
            
            if (isScreen) {
                j2me::platform::GraphicsContext::getInstance().setClip(x, y, w, h);
            } else if (target) {
                SDL_Rect rect = {x, y, w, h};
                SDL_SetClipRect(target, &rect);
            }
        }
    );

    // Stub: javax/microedition/lcdui/Graphics.drawRoundRectNative(IIIIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "drawRoundRectNative", "(IIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // arcHeight
            frame->pop(); // arcWidth
            frame->pop(); // height
            frame->pop(); // width
            frame->pop(); // y
            frame->pop(); // x
            frame->pop(); // this
            // TODO: Implement drawRoundRect
        }
    );

    // Stub: javax/microedition/lcdui/Graphics.fillRoundRectNative(IIIIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "fillRoundRectNative", "(IIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // arcHeight
            frame->pop(); // arcWidth
            frame->pop(); // height
            frame->pop(); // width
            frame->pop(); // y
            frame->pop(); // x
            frame->pop(); // this
            // TODO: Implement fillRoundRect
        }
    );

    // Stub: javax/microedition/lcdui/Graphics.fillArcNative(IIIIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "fillArcNative", "(IIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // arcAngle
            frame->pop(); // startAngle
            frame->pop(); // height
            frame->pop(); // width
            frame->pop(); // y
            frame->pop(); // x
            frame->pop(); // this
            // TODO: Implement fillArc
        }
    );

    // Stub: javax/microedition/lcdui/Graphics.drawArcNative(IIIIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "drawArcNative", "(IIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // arcAngle
            frame->pop(); // startAngle
            frame->pop(); // height
            frame->pop(); // width
            frame->pop(); // y
            frame->pop(); // x
            frame->pop(); // this
            // TODO: Implement drawArc
        }
    );

    // Stub: javax/microedition/lcdui/Graphics.drawRegionNative(Ljavax/microedition/lcdui/Image;IIIIIIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "drawRegionNative", "(Ljavax/microedition/lcdui/Image;IIIIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // anchor
            frame->pop(); // y_dest
            frame->pop(); // x_dest
            frame->pop(); // transform
            frame->pop(); // height
            frame->pop(); // width
            frame->pop(); // y_src
            frame->pop(); // x_src
            frame->pop(); // src image
            frame->pop(); // this
            // TODO: Implement drawRegion
        }
    );

    // Stub: javax/microedition/lcdui/Graphics.copyAreaNative(IIIIIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "copyAreaNative", "(IIIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // anchor
            frame->pop(); // y_dest
            frame->pop(); // x_dest
            frame->pop(); // height
            frame->pop(); // width
            frame->pop(); // y_src
            frame->pop(); // x_src
            frame->pop(); // this
            // TODO: Implement copyArea
        }
    );

    // Stub: javax/microedition/lcdui/Graphics.fillTriangleNative(IIIIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "fillTriangleNative", "(IIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // y3
            frame->pop(); // x3
            frame->pop(); // y2
            frame->pop(); // x2
            frame->pop(); // y1
            frame->pop(); // x1
            frame->pop(); // this
            // TODO: Implement fillTriangle
        }
    );

    // Stub: javax/microedition/lcdui/Graphics.drawRGBNative([IIIIIIIZ)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "drawRGBNative", "([IIIIIIIZ)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // processAlpha
            frame->pop(); // height
            frame->pop(); // width
            frame->pop(); // y
            frame->pop(); // x
            frame->pop(); // scanlength
            frame->pop(); // offset
            frame->pop(); // rgbData
            frame->pop(); // this
            // TODO: Implement drawRGB
        }
    );

    // Stub: javax/microedition/lcdui/Graphics.translateNative(II)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "translateNative", "(II)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // y
            frame->pop(); // x
            frame->pop(); // this
            // Translation is handled in Java layer for now
        }
    );
}

} // namespace natives
} // namespace j2me
