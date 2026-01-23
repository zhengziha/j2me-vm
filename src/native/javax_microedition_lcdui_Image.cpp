#include "javax_microedition_lcdui_Image.hpp"
#include "ImageCommon.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../platform/GraphicsContext.hpp"
#include "../platform/stb_image.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <map>
#include <iomanip>

namespace j2me {
namespace natives {

void registerImageNatives() {
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
                        std::cout << "[Image] File found. Size: " << data->size() << " bytes." << std::endl;
                        SDL_Surface* surface = j2me::platform::GraphicsContext::getInstance().createImage(data->data(), data->size());
                        if (surface) {
                            imgId = nextImageId++;
                            imageMap[imgId] = surface;
                            std::cout << "[Image] Loaded successfully, ID: " << imgId << " Size: " << surface->w << "x" << surface->h << std::endl;
                        } else {
                            std::cerr << "[Image] Failed to decode image: " << resName << std::endl;
                            // Print first few bytes for debugging
                            std::cerr << "Header bytes: ";
                            const unsigned char* bytes = data->data();
                            for (size_t i = 0; i < std::min((size_t)16, data->size()); ++i) {
                                std::cerr << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i] << " ";
                            }
                            std::cerr << std::dec << std::endl;
                        }
                    } else {
                        std::cerr << "[Image] Failed to read file data" << std::endl;
                    }
                } else {
                    std::cerr << "[Image] Image file not found in JAR: " << resName << std::endl;
                }
            }
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = imgId;
            frame->push(result);
        }
    );

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

    // javax/microedition/lcdui/Image.getRGBNative(I[IIIIIII)V
    registry.registerNative("javax/microedition/lcdui/Image", "getRGBNative", "(I[IIIIIII)V", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int height = frame->pop().val.i;
            int width = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            int scanlength = frame->pop().val.i;
            int offset = frame->pop().val.i;
            j2me::core::JavaValue rgbDataVal = frame->pop();
            int imgId = frame->pop().val.i;
            frame->pop(); // this
            
            if (rgbDataVal.type == j2me::core::JavaValue::REFERENCE && rgbDataVal.val.ref != nullptr) {
                auto rgbArray = static_cast<j2me::core::JavaObject*>(rgbDataVal.val.ref);
                
                auto it = imageMap.find(imgId);
                if (it != imageMap.end()) {
                    SDL_Surface* surface = it->second;
                    SDL_LockSurface(surface);
                    uint32_t* pixels = (uint32_t*)surface->pixels;
                    
                    for (int r = 0; r < height; r++) {
                        for (int c = 0; c < width; c++) {
                            if (x + c >= 0 && x + c < surface->w && y + r >= 0 && y + r < surface->h) {
                                uint32_t pixel = pixels[(y + r) * surface->w + (x + c)];
                                
                                // Convert ARGB8888 to ARGB (Java format)
                                // SDL_PIXELFORMAT_ARGB8888 is generally compatible with Java ARGB
                                // But let's ensure alpha is preserved.
                                
                                int targetIndex = offset + r * scanlength + c;
                                if (targetIndex >= 0 && targetIndex < (int)rgbArray->fields.size()) {
                                    rgbArray->fields[targetIndex] = pixel;
                                }
                            }
                        }
                    }
                    
                    SDL_UnlockSurface(surface);
                    std::cout << "[Image] getRGBNative executed" << std::endl;
                } else {
                    std::cerr << "[Image] getRGBNative: Invalid Image ID " << imgId << std::endl;
                }
            } else {
                 std::cerr << "[Image] getRGBNative: rgbData is null" << std::endl;
            }
        }
    );

    // javax/microedition/lcdui/Image.createMutableImageNative(II)I
    registry.registerNative("javax/microedition/lcdui/Image", "createMutableImageNative", "(II)I", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int height = frame->pop().val.i;
            int width = frame->pop().val.i;
            
            // Create a new SDL Surface
            // Use ARGB8888 for consistency
            SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
            
            if (surface) {
                // Fill with white (usually mutable images start white)
                SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 255, 255, 255, 255));
                
                int32_t imgId = nextImageId++;
                imageMap[imgId] = surface;
                
                j2me::core::JavaValue result;
                result.type = j2me::core::JavaValue::INT;
                result.val.i = imgId;
                frame->push(result);
                
                std::cout << "[Image] Created Mutable Image, ID: " << imgId << " Size: " << width << "x" << height << std::endl;
            } else {
                std::cerr << "[Image] Failed to create mutable image: " << SDL_GetError() << std::endl;
                // Return -1 or 0? 0 is probably safer as it might be checked.
                // Assuming ID 0 is reserved or invalid if nextImageId starts at 1.
                // Let's verify nextImageId init.
                j2me::core::JavaValue result;
                result.type = j2me::core::JavaValue::INT;
                result.val.i = 0; 
                frame->push(result);
            }
        }
    );

    // javax/microedition/lcdui/Image.createImageFromDataNative([BII)I
    registry.registerNative("javax/microedition/lcdui/Image", "createImageFromDataNative", "([BII)I", 
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            int length = frame->pop().val.i;
            int offset = frame->pop().val.i;
            j2me::core::JavaValue dataVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = 0;
            
            if (dataVal.type == j2me::core::JavaValue::REFERENCE && dataVal.val.ref != nullptr) {
                auto dataObj = static_cast<j2me::core::JavaObject*>(dataVal.val.ref);
                
                // Validate bounds
                if (offset < 0 || length < 0 || offset + length > (int)dataObj->fields.size()) {
                     std::cerr << "IndexOutOfBoundsException in createImageFromData" << std::endl;
                     frame->push(result);
                     return;
                }
                
                // Extract data to a temporary buffer
                std::vector<unsigned char> buffer(length);
                for (int i = 0; i < length; i++) {
                    buffer[i] = (unsigned char)dataObj->fields[offset + i];
                }
                
                // Debug: Print header
                // std::cout << "[Image] createImageFromData length=" << length << " offset=" << offset << std::endl;
                /*
                if (length > 0) {
                    std::cout << "[Image] createImageFromData length=" << length << " Header: ";
                    for (int i = 0; i < std::min(16, length); i++) {
                        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i] << " ";
                    }
                    std::cout << std::dec << std::endl;
                }
                */

                // Load with stbi
                int w, h, channels;
                // Force 4 channels (RGBA)
                unsigned char* pixels = stbi_load_from_memory(buffer.data(), length, &w, &h, &channels, 4);
                
                if (pixels) {
                    // Create SDL Surface
                    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
                    if (surface) {
                        SDL_LockSurface(surface);
                        
                        uint32_t* targetPixels = (uint32_t*)surface->pixels;
                        for (int i = 0; i < w * h; i++) {
                            unsigned char r = pixels[i*4 + 0];
                            unsigned char g = pixels[i*4 + 1];
                            unsigned char b = pixels[i*4 + 2];
                            unsigned char a = pixels[i*4 + 3];
                            
                            targetPixels[i] = SDL_MapRGBA(surface->format, r, g, b, a);
                        }
                        
                        SDL_UnlockSurface(surface);
                        
                        int32_t imgId = nextImageId++;
                        imageMap[imgId] = surface;
                        
                        result.val.i = imgId;
                        std::cout << "[Image] Created Immutable Image (from data), ID: " << imgId << " Size: " << w << "x" << h << std::endl;
                    } else {
                         std::cerr << "Failed to create SDL surface" << std::endl;
                    }
                    
                    stbi_image_free(pixels);
                } else {
                    std::cerr << "Failed to decode image data" << std::endl;
                }
            } else {
                 std::cerr << "NullPointerException in createImageFromData" << std::endl;
            }
            frame->push(result);
        }
    );
}

} // namespace natives
} // namespace j2me
