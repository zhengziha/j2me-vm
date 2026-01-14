#include "javax_microedition_lcdui_Image.hpp"
#include "ImageCommon.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include <SDL2/SDL.h>
#include <iostream>
#include <map>

namespace j2me {
namespace natives {

void registerImageNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();

    // javax/microedition/lcdui/Image.getWidth()I
    registry.registerNative("javax/microedition/lcdui/Image", "getWidth", "()I", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = 0;
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* imgObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (imgObj->fields.size() > 0) {
                    int32_t imgId = (int32_t)imgObj->fields[0];
                    auto it = imageMap.find(imgId);
                    if (it != imageMap.end()) {
                        SDL_Surface* surface = it->second;
                        result.val.i = surface->w;
                        std::cout << "[Image] getWidth: " << result.val.i << std::endl;
                    } else {
                        std::cerr << "[Image] getWidth: Invalid Image ID " << imgId << std::endl;
                    }
                }
            }
            
            frame->push(result);
        }
    );

    // javax/microedition/lcdui/Image.getHeight()I
    registry.registerNative("javax/microedition/lcdui/Image", "getHeight", "()I", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = 0;
            
            if (thisVal.type == j2me::core::JavaValue::REFERENCE && thisVal.val.ref != nullptr) {
                j2me::core::JavaObject* imgObj = (j2me::core::JavaObject*)thisVal.val.ref;
                if (imgObj->fields.size() > 0) {
                    int32_t imgId = (int32_t)imgObj->fields[0];
                    auto it = imageMap.find(imgId);
                    if (it != imageMap.end()) {
                        SDL_Surface* surface = it->second;
                        result.val.i = surface->h;
                        std::cout << "[Image] getHeight: " << result.val.i << std::endl;
                    } else {
                        std::cerr << "[Image] getHeight: Invalid Image ID " << imgId << std::endl;
                    }
                }
            }
            
            frame->push(result);
        }
    );
}

} // namespace natives
} // namespace j2me
