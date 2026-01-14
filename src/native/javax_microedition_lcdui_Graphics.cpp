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

void registerGraphicsNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();

    // javax/microedition/lcdui/Image.createImageNative(Ljava/lang/String;)I
    registry.registerNative("javax/microedition/lcdui/Image", "createImageNative", "(Ljava/lang/String;)I", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop(); // name string
            
            int32_t imgId = 0;
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && !nameVal.strVal.empty()) {
                std::string resName = nameVal.strVal;
                // Remove leading slash if present
                if (resName.size() > 0 && resName[0] == '/') resName = resName.substr(1);
                
                std::cout << "[Image] Loading image: " << resName << std::endl;
                
                auto loader = j2me::core::NativeRegistry::getInstance().getJarLoader();
                if (loader && loader->hasFile(resName)) {
                    auto data = loader->getFile(resName);
                    if (data) {
                        SDL_Surface* surface = j2me::platform::GraphicsContext::getInstance().createImage(data->data(), data->size());
                        if (surface) {
                            imgId = nextImageId++;
                            imageMap[imgId] = surface;
                            std::cout << "[Image] Loaded successfully, ID: " << imgId << " Size: " << surface->w << "x" << surface->h << std::endl;
                        } else {
                            std::cerr << "[Image] Failed to decode image" << std::endl;
                        }
                    } else {
                        std::cerr << "[Image] Failed to read file data" << std::endl;
                    }
                } else {
                    std::cerr << "[Image] File not found in JAR: " << resName << std::endl;
                    // Fallback to Mock for testing if file missing but we want to survive
                    int w = 32, h = 32;
                    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
                    if (surface) {
                        SDL_LockSurface(surface);
                        uint32_t* pixels = (uint32_t*)surface->pixels;
                        for(int y=0; y<h; y++) {
                            for(int x=0; x<w; x++) {
                                pixels[y*w+x] = SDL_MapRGBA(surface->format, x*8, y*8, 128, 255);
                            }
                        }
                        SDL_UnlockSurface(surface);
                        imgId = nextImageId++;
                        imageMap[imgId] = surface;
                        std::cout << "[Image] Created Mock Image, ID: " << imgId << std::endl;
                    }
                }
            }
            
            // Return int ID
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = imgId;
            frame->push(ret);
        }
    );

    // javax/microedition/lcdui/Graphics.drawImage(Ljavax/microedition/lcdui/Image;III)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "drawImage", "(Ljavax/microedition/lcdui/Image;III)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int anchor = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            j2me::core::JavaValue imgVal = frame->pop();
            frame->pop(); // this
            
            if (imgVal.val.ref != nullptr) {
                // Get nativePtr field
                j2me::core::JavaObject* imgObj = (j2me::core::JavaObject*)imgVal.val.ref;
                
                if (imgObj->fields.size() > 0) {
                     int32_t imgId = (int32_t)imgObj->fields[0];
                     if (imgId != 0) {
                         auto it = imageMap.find(imgId);
                         if (it != imageMap.end()) {
                             SDL_Surface* surface = it->second;
                             static int frameCount = 0;
                             if (frameCount++ % 60 == 0) { // Log once per second (approx)
                                 std::cout << "[Graphics] drawImage ID: " << imgId << " at " << x << "," << y << std::endl;
                             }
                             j2me::platform::GraphicsContext::getInstance().drawImage(surface, x, y);
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

    // javax/microedition/lcdui/Graphics.drawLine(IIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "drawLine", "(IIII)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int y2 = frame->pop().val.i;
            int x2 = frame->pop().val.i;
            int y1 = frame->pop().val.i;
            int x1 = frame->pop().val.i;
            frame->pop(); // this (Graphics object)
            
            j2me::platform::GraphicsContext::getInstance().drawLine(x1, y1, x2, y2);
        }
    );

    // javax/microedition/lcdui/Graphics.fillRect(IIII)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "fillRect", "(IIII)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int h = frame->pop().val.i;
            int w = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            frame->pop(); // this
            
            j2me::platform::GraphicsContext::getInstance().fillRect(x, y, w, h);
        }
    );

    // javax/microedition/lcdui/Graphics.setColor(III)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "setColor", "(III)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int b = frame->pop().val.i;
            int g = frame->pop().val.i;
            int r = frame->pop().val.i;
            frame->pop(); // this
            
            j2me::platform::GraphicsContext::getInstance().setColor(r, g, b);
        }
    );
    
    // javax/microedition/lcdui/Graphics.drawString(Ljava/lang/String;III)V
    registry.registerNative("javax/microedition/lcdui/Graphics", "drawString", "(Ljava/lang/String;III)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int anchor = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            j2me::core::JavaValue strVal = frame->pop();
            frame->pop(); // this
            
            if (strVal.type == j2me::core::JavaValue::REFERENCE && !strVal.strVal.empty()) {
                // For now, just print to console for debugging
                std::cout << "[Graphics] drawString: \"" << strVal.strVal << "\" at (" << x << "," << y << ")" << std::endl;
                
                // TODO: Implement actual text rendering (e.g. using a simple bitmap font)
                // For now, let's draw a small rectangle to indicate text position
                j2me::platform::GraphicsContext::getInstance().fillRect(x, y, 50, 10);
            }
        }
    );
}

} // namespace natives
} // namespace j2me
