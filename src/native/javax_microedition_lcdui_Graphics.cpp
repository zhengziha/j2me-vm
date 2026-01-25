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

static void applyGraphicsColor(j2me::core::JavaObject* graphicsObj) {
    uint32_t color = getGraphicsColor(graphicsObj);
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = (color) & 0xFF;
    j2me::platform::GraphicsContext::getInstance().setColor(r, g, b);
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
                                 // std::cout << "[Graphics] drawImage to Screen: src=" << imgId << " pos=" << x << "," << y << " srcSize=" << srcSurface->w << "x" << srcSurface->h << std::endl;
                                 j2me::platform::GraphicsContext::getInstance().drawImage(srcSurface, x, y, anchor);
                             } else if (target) {
                                 // Draw to offscreen surface
                                 // std::cout << "[Graphics] drawImage to Offscreen: src=" << imgId << " pos=" << x << "," << y << " srcSize=" << srcSurface->w << "x" << srcSurface->h << std::endl;
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

            applyGraphicsColor(graphicsObj);

            if (isScreen) {
                j2me::platform::GraphicsContext::getInstance().drawLine(x1, y1, x2, y2);
            } else if (target) {
                j2me::platform::GraphicsContext::getInstance().drawLine(x1, y1, x2, y2, target);
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
            
            applyGraphicsColor(graphicsObj);

            if (isScreen) {
                j2me::platform::GraphicsContext::getInstance().fillRect(x, y, w, h);
            } else if (target) {
                j2me::platform::GraphicsContext::getInstance().fillRect(x, y, w, h, target);
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
            
            applyGraphicsColor(graphicsObj);

            auto& gc = j2me::platform::GraphicsContext::getInstance();
            SDL_Surface* t = isScreen ? nullptr : target;
            
            if (isScreen || target) {
                gc.drawLine(x, y, x + w - 1, y, t);
                gc.drawLine(x, y + h - 1, x + w - 1, y + h - 1, t);
                gc.drawLine(x, y, x, y + h - 1, t);
                gc.drawLine(x + w - 1, y, x + w - 1, y + h - 1, t);
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
            int arcHeight = frame->pop().val.i;
            int arcWidth = frame->pop().val.i;
            int height = frame->pop().val.i;
            int width = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);
            applyGraphicsColor(graphicsObj);
            
            if (isScreen) {
                j2me::platform::GraphicsContext::getInstance().drawRoundRect(x, y, width, height, arcWidth, arcHeight);
            } else if (target) {
                j2me::platform::GraphicsContext::getInstance().drawRoundRect(x, y, width, height, arcWidth, arcHeight, target);
            }
        }
    );

    // Stub: javax/microedition/lcdui/Graphics.fillRoundRectNative(IIIIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "fillRoundRectNative", "(IIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int arcHeight = frame->pop().val.i;
            int arcWidth = frame->pop().val.i;
            int height = frame->pop().val.i;
            int width = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);
            applyGraphicsColor(graphicsObj);
            
            if (isScreen) {
                j2me::platform::GraphicsContext::getInstance().fillRoundRect(x, y, width, height, arcWidth, arcHeight);
            } else if (target) {
                j2me::platform::GraphicsContext::getInstance().fillRoundRect(x, y, width, height, arcWidth, arcHeight, target);
            }
        }
    );

    // Stub: javax/microedition/lcdui/Graphics.fillArcNative(IIIIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "fillArcNative", "(IIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int arcAngle = frame->pop().val.i;
            int startAngle = frame->pop().val.i;
            int height = frame->pop().val.i;
            int width = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);
            applyGraphicsColor(graphicsObj);
            
            if (isScreen) {
                j2me::platform::GraphicsContext::getInstance().fillArc(x, y, width, height, startAngle, arcAngle);
            } else if (target) {
                j2me::platform::GraphicsContext::getInstance().fillArc(x, y, width, height, startAngle, arcAngle, target);
            }
        }
    );

    // Stub: javax/microedition/lcdui/Graphics.drawArcNative(IIIIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "drawArcNative", "(IIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int arcAngle = frame->pop().val.i;
            int startAngle = frame->pop().val.i;
            int height = frame->pop().val.i;
            int width = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);
            applyGraphicsColor(graphicsObj);
            
            if (isScreen) {
                j2me::platform::GraphicsContext::getInstance().drawArc(x, y, width, height, startAngle, arcAngle);
            } else if (target) {
                j2me::platform::GraphicsContext::getInstance().drawArc(x, y, width, height, startAngle, arcAngle, target);
            }
        }
    );

    // javax/microedition/lcdui/Graphics.drawRegionNative(Ljavax/microedition/lcdui/Image;IIIIIIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "drawRegionNative", "(Ljavax/microedition/lcdui/Image;IIIIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int anchor = frame->pop().val.i;
            int y_dest = frame->pop().val.i;
            int x_dest = frame->pop().val.i;
            int transform = frame->pop().val.i;
            int height = frame->pop().val.i;
            int width = frame->pop().val.i;
            int y_src = frame->pop().val.i;
            int x_src = frame->pop().val.i;
            j2me::core::JavaValue imgVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop(); // this
            
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);
            
            if (imgVal.val.ref != nullptr) {
                j2me::core::JavaObject* imgObj = (j2me::core::JavaObject*)imgVal.val.ref;
                if (imgObj->fields.size() > 0) {
                     int32_t imgId = (int32_t)imgObj->fields[0];
                     if (imgId != 0) {
                         auto it = imageMap.find(imgId);
                         if (it != imageMap.end()) {
                             SDL_Surface* srcSurface = it->second;
                            if (isScreen) {
                                // std::cout << "[Graphics] drawRegion to Screen: src=" << imgId << " pos=" << x_dest << "," << y_dest << " size=" << width << "x" << height << " trans=" << transform << std::endl;
                                j2me::platform::GraphicsContext::getInstance().drawRegion(srcSurface, x_src, y_src, width, height, transform, x_dest, y_dest, anchor);
                            } else if (target) {
                                // std::cout << "[Graphics] drawRegion to Offscreen: src=" << imgId << " pos=" << x_dest << "," << y_dest << " size=" << width << "x" << height << " trans=" << transform << std::endl;
                                j2me::platform::GraphicsContext::getInstance().drawRegion(srcSurface, x_src, y_src, width, height, transform, x_dest, y_dest, anchor, target);
                            }
                         }
                     }
                }
            }
        }
    );

    // Stub: javax/microedition/lcdui/Graphics.copyAreaNative(IIIIIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "copyAreaNative", "(IIIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int anchor = frame->pop().val.i;
            int y_dest = frame->pop().val.i;
            int x_dest = frame->pop().val.i;
            int height = frame->pop().val.i;
            int width = frame->pop().val.i;
            int y_src = frame->pop().val.i;
            int x_src = frame->pop().val.i;
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);
            
            if (isScreen) {
                j2me::platform::GraphicsContext::getInstance().copyArea(x_src, y_src, width, height, x_dest, y_dest, anchor);
            } else if (target) {
                j2me::platform::GraphicsContext::getInstance().copyArea(x_src, y_src, width, height, x_dest, y_dest, anchor, target);
            }
        }
    );

    // Stub: javax/microedition/lcdui/Graphics.fillTriangleNative(IIIIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "fillTriangleNative", "(IIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int y3 = frame->pop().val.i;
            int x3 = frame->pop().val.i;
            int y2 = frame->pop().val.i;
            int x2 = frame->pop().val.i;
            int y1 = frame->pop().val.i;
            int x1 = frame->pop().val.i;
            j2me::core::JavaValue thisVal = frame->pop();
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);
            applyGraphicsColor(graphicsObj);
            
            if (isScreen) {
                j2me::platform::GraphicsContext::getInstance().fillTriangle(x1, y1, x2, y2, x3, y3);
            } else if (target) {
                j2me::platform::GraphicsContext::getInstance().fillTriangle(x1, y1, x2, y2, x3, y3, target);
            }
        }
    );

    // javax/microedition/lcdui/Graphics.drawRGBNative([IIIIIIIZ)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "drawRGBNative", "([IIIIIIIZ)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int processAlpha = frame->pop().val.i;
            int height = frame->pop().val.i;
            int width = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            int scanlength = frame->pop().val.i;
            int offset = frame->pop().val.i;
            j2me::core::JavaValue rgbDataVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop(); // this
            
            j2me::core::JavaObject* graphicsObj = (j2me::core::JavaObject*)thisVal.val.ref;
            bool isScreen;
            SDL_Surface* target = getTargetSurface(graphicsObj, isScreen);

            if (rgbDataVal.val.ref != nullptr) {
                auto rgbArray = static_cast<j2me::core::JavaObject*>(rgbDataVal.val.ref);
                if (rgbArray->fields.size() > 0) {
                     int64_t* rgbData = rgbArray->fields.data();
                     if (isScreen) {
                         // std::cout << "[Graphics] drawRGB to Screen: pos=" << x << "," << y << " size=" << width << "x" << height << " alpha=" << processAlpha << std::endl;
                         j2me::platform::GraphicsContext::getInstance().drawRGB(rgbData, offset, scanlength, x, y, width, height, processAlpha != 0);
                     } else if (target) {
                         // std::cout << "[Graphics] drawRGB to Offscreen: pos=" << x << "," << y << " size=" << width << "x" << height << " alpha=" << processAlpha << std::endl;
                         j2me::platform::GraphicsContext::getInstance().drawRGB(rgbData, offset, scanlength, x, y, width, height, processAlpha != 0, target);
                     }
                }
            }
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
