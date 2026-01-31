#include "javax_microedition_lcdui_Image.hpp"
#include "ImageCommon.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../platform/GraphicsContext.hpp"
#include "java_lang_String.hpp"
#include "../platform/stb_image.h"
#include "../core/Diagnostics.hpp"
#include "../core/Logger.hpp"
#include <SDL2/SDL.h>
#include <map>
#include <iomanip>

namespace j2me {
namespace natives {

void registerImageNatives(j2me::core::NativeRegistry& registry) {
    // registry passed as argument

    // javax/microedition/lcdui/Image.createImage(Ljava/io/InputStream;)Ljavax/microedition/lcdui/Image;
    registry.registerNative("javax/microedition/lcdui/Image", "createImageNative", "(Ljava/lang/String;)I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop(); // name string
            
            int32_t imgId = 0;
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && nameVal.val.ref != nullptr) {
                j2me::core::JavaObject* strObj = (j2me::core::JavaObject*)nameVal.val.ref;
                std::string resName = getJavaString(strObj);
                
                // Remove leading slash if present
                if (resName.size() > 0 && resName[0] == '/') resName = resName.substr(1);
                
                auto loader = j2me::core::NativeRegistry::getInstance().getJarLoader();
                if (loader && loader->hasFile(resName)) {
                    auto data = loader->getFile(resName);
                    if (data) {
                        LOG_DEBUG("[Image] File found. Size: " + std::to_string(data->size()) + " bytes.");
                        SDL_Surface* surface = j2me::platform::GraphicsContext::getInstance().createImage(data->data(), data->size());
                        if (surface) {
                            imgId = nextImageId++;
                            imageMap[imgId] = surface;
                            LOG_DEBUG("[Image] Loaded successfully, ID: " + std::to_string(imgId) + " Size: " + std::to_string(surface->w) + "x" + std::to_string(surface->h));
                        } else {
                            LOG_ERROR("[Image] Failed to decode image: " + resName);
                            // Print first few bytes for debugging
                            std::string headerHex;
                            const unsigned char* bytes = data->data();
                            for (size_t i = 0; i < std::min((size_t)16, data->size()); ++i) {
                                char buf[8];
                                snprintf(buf, sizeof(buf), "%02X", bytes[i]);
                                headerHex += buf;
                            }
                            LOG_ERROR("[Image] Header bytes: " + headerHex);
                            j2me::core::Diagnostics::getInstance().onImageDecodeFailed(resName, headerHex);
                        }
                    } else {
                        LOG_ERROR("[Image] Failed to read file data");
                    }
                } else {
                    LOG_ERROR("[Image] Image file not found in JAR: " + resName);
                    j2me::core::Diagnostics::getInstance().onResourceNotFound(resName);
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
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
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
                    } else {
                        LOG_ERROR("[Image] getWidth: Invalid Image ID " + std::to_string(imgId));
                    }
                }
            }
            
            frame->push(result);
        }
    );

    // javax/microedition/lcdui/Image.getHeight()I
    registry.registerNative("javax/microedition/lcdui/Image", "getHeight", "()I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
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
                    } else {
                        LOG_ERROR("[Image] getHeight: Invalid Image ID " + std::to_string(imgId));
                    }
                }
            }
            
            frame->push(result);
        }
    );

    // javax/microedition/lcdui/Image.getRGB([IIIIIII)V
    registry.registerNative("javax/microedition/lcdui/Image", "getRGB", "([IIIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int height = frame->pop().val.i;
            int width = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            int scanlength = frame->pop().val.i;
            int offset = frame->pop().val.i;
            j2me::core::JavaValue rgbDataVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            if (thisVal.type != j2me::core::JavaValue::REFERENCE || thisVal.val.ref == nullptr) {
                LOG_ERROR("[Image] getRGB: this is null");
                return;
            }
            j2me::core::JavaObject* imgObj = (j2me::core::JavaObject*)thisVal.val.ref;
            int32_t imgId = 0;
            if (imgObj->fields.size() > 0) {
                imgId = (int32_t)imgObj->fields[0];
            }
            
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
                                uint8_t pr, pg, pb, pa;
                                SDL_GetRGBA(pixel, surface->format, &pr, &pg, &pb, &pa);
                                uint32_t argb = ((uint32_t)pa << 24) | ((uint32_t)pr << 16) | ((uint32_t)pg << 8) | (uint32_t)pb;
                                
                                int targetIndex = offset + r * scanlength + c;
                                if (targetIndex >= 0 && targetIndex < (int)rgbArray->fields.size()) {
                                    rgbArray->fields[targetIndex] = argb;
                                }
                            }
                        }
                    }
                    
                    SDL_UnlockSurface(surface);
                } else {
                    LOG_ERROR("[Image] getRGB: Invalid Image ID " + std::to_string(imgId));
                }
            } else {
                 LOG_ERROR("[Image] getRGB: rgbData is null");
            }
        }
    );

    // javax/microedition/lcdui/Image.getRGBNative(I[IIIIIII)V
    registry.registerNative("javax/microedition/lcdui/Image", "getRGBNative", "(I[IIIIIII)V", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
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
                                uint8_t pr, pg, pb, pa;
                                SDL_GetRGBA(pixel, surface->format, &pr, &pg, &pb, &pa);
                                uint32_t argb = ((uint32_t)pa << 24) | ((uint32_t)pr << 16) | ((uint32_t)pg << 8) | (uint32_t)pb;
                                
                                int targetIndex = offset + r * scanlength + c;
                                if (targetIndex >= 0 && targetIndex < (int)rgbArray->fields.size()) {
                                    rgbArray->fields[targetIndex] = argb;
                                }
                            }
                        }
                    }
                    
                    SDL_UnlockSurface(surface);
                    LOG_DEBUG("[Image] getRGBNative executed");
                } else {
                    LOG_ERROR("[Image] getRGBNative: Invalid Image ID " + std::to_string(imgId));
                }
            } else {
                 LOG_ERROR("[Image] getRGBNative: rgbData is null");
            }
        }
    );

    // javax/microedition/lcdui/Image.createRGBImageNative([IIIZ)I
    registry.registerNative("javax/microedition/lcdui/Image", "createRGBImageNative", "([IIIZ)I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            LOG_DEBUG("[Image] createRGBImageNative called");
            int processAlpha = frame->pop().val.i;
            int height = frame->pop().val.i;
            int width = frame->pop().val.i;
            j2me::core::JavaValue rgbDataVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = 0;
            
            if (rgbDataVal.type == j2me::core::JavaValue::REFERENCE && rgbDataVal.val.ref != nullptr) {
                auto rgbArray = static_cast<j2me::core::JavaObject*>(rgbDataVal.val.ref);
                
                // Validate array size
                if (rgbArray->fields.size() < (size_t)(width * height)) {
                     LOG_ERROR("ArrayIndexOutOfBoundsException in createRGBImageNative");
                     frame->push(result);
                     return;
                }
                
                SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
                if (surface) {
                    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
                    SDL_LockSurface(surface);
                    uint32_t* targetPixels = (uint32_t*)surface->pixels;
                    
                    for (int i = 0; i < width * height; i++) {
                        uint32_t argb = (uint32_t)rgbArray->fields[i];
                        uint8_t a = (argb >> 24) & 0xFF;
                        uint8_t r = (argb >> 16) & 0xFF;
                        uint8_t g = (argb >> 8) & 0xFF;
                        uint8_t b = (argb) & 0xFF;
                        if (!processAlpha) a = 255;
                        targetPixels[i] = SDL_MapRGBA(surface->format, r, g, b, a);
                    }
                    
                    SDL_UnlockSurface(surface);
                    
                    int32_t imgId = nextImageId++;
                    imageMap[imgId] = surface;
                    result.val.i = imgId;
                    
                    LOG_DEBUG("[Image] Created RGB Image, ID: " + std::to_string(imgId) + " Size: " + std::to_string(width) + "x" + std::to_string(height));
                }
            }
            frame->push(result);
        }
    );

    // javax/microedition/lcdui/Image.createMutableImageNative(II)I
    registry.registerNative("javax/microedition/lcdui/Image", "createMutableImageNative", "(II)I", 
        [](std::shared_ptr<j2me::core::JavaThread>, std::shared_ptr<j2me::core::StackFrame> frame) {
            int height = frame->pop().val.i;
            int width = frame->pop().val.i;
            
            if (width <= 0) {
                LOG_DEBUG("[Image] Warning: createMutableImageNative called with width=" + std::to_string(width) + ", forcing to 240");
                width = 240;
            }
            if (height <= 0) {
                LOG_DEBUG("[Image] Warning: createMutableImageNative called with height=" + std::to_string(height) + ", forcing to 320");
                height = 320;
            }

            SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
            
            if (surface) {
                SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
                SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 255, 255, 255, 255));
                
                int32_t imgId = nextImageId++;
                imageMap[imgId] = surface;
                mutableImages.insert(imgId);
                
                j2me::core::JavaValue result;
                result.type = j2me::core::JavaValue::INT;
                result.val.i = imgId;
                frame->push(result);
                
                LOG_DEBUG("[Image] Created Mutable Image, ID: " + std::to_string(imgId) + " Size: " + std::to_string(width) + "x" + std::to_string(height));
            } else {
                LOG_ERROR("[Image] Failed to create mutable image: " + std::string(SDL_GetError()));
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

    // javax/microedition/lcdui/Image.isMutableNative(I)Z
    registry.registerNative("javax/microedition/lcdui/Image", "isMutableNative", "(I)Z", 
        [](std::shared_ptr<j2me::core::JavaThread>, std::shared_ptr<j2me::core::StackFrame> frame) {
            int32_t imgId = frame->pop().val.i;
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = (mutableImages.find(imgId) != mutableImages.end()) ? 1 : 0;
            
            frame->push(result);
        }
    );

    // javax/microedition/lcdui/Image.createImageFromDataNative([BII)I
    registry.registerNative("javax/microedition/lcdui/Image", "createImageFromDataNative", "([BII)I", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
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
                     LOG_ERROR("IndexOutOfBoundsException in createImageFromData");
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
                    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
                    if (surface) {
                        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
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
                        LOG_DEBUG("[Image] Created Immutable Image (from data), ID: " + std::to_string(imgId) + " Size: " + std::to_string(w) + "x" + std::to_string(h));
                    } else {
                         LOG_ERROR("Failed to create SDL surface: " + std::string(SDL_GetError()));
                    }
                    
                    stbi_image_free(pixels);
                } else {
                    LOG_ERROR("Failed to decode image data");
                    std::string headerHex;
                    for (int i = 0; i < std::min(16, length); i++) {
                        char buf[8];
                        snprintf(buf, sizeof(buf), "%02X", buffer[i]);
                        headerHex += buf;
                    }
                    j2me::core::Diagnostics::getInstance().onImageDecodeFailed("<data>", headerHex);
                }
            } else {
                 LOG_ERROR("NullPointerException in createImageFromData");
            }
            frame->push(result);
        }
    );

    // javax/microedition/lcdui/Image.getGraphicsNative(I)I
    registry.registerNative("javax/microedition/lcdui/Image", "getGraphicsNative", "(I)I", 
        [](std::shared_ptr<j2me::core::JavaThread>, std::shared_ptr<j2me::core::StackFrame> frame) {
            int32_t imgId = frame->pop().val.i;
            // For now, return the same ID, assuming 1:1 mapping between Image and Graphics context for offscreen
            // Real implementation would need a Graphics object tracking this target
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = imgId; // Just return image ID as graphics ID
            frame->push(result);
        }
    );
}

} // namespace natives
} // namespace j2me
