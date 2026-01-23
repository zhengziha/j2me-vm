#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
// #define STB_IMAGE_IMPLEMENTATION // Removed from header!
#include "stb_image.h"
#include <memory>
#include <iostream>
#include <mutex>

namespace j2me {
namespace platform {

class GraphicsContext {
public:
    static GraphicsContext& getInstance() {
        static GraphicsContext instance;
        return instance;
    }

    void init(SDL_Window* window) {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        this->window = window;
        
        // Window Surface (Screen)
        windowSurface = SDL_GetWindowSurface(window);
        
        // Back Buffer (Drawing Target)
        surface = SDL_CreateRGBSurfaceWithFormat(0, windowSurface->w, windowSurface->h, 32, windowSurface->format->format);
        
        // Front Buffer (Display Target)
        displaySurface = SDL_CreateRGBSurfaceWithFormat(0, windowSurface->w, windowSurface->h, 32, windowSurface->format->format);
        
        // Fill white
        SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 255, 255));
        SDL_FillRect(displaySurface, nullptr, SDL_MapRGB(displaySurface->format, 255, 255, 255));
        
        // Initial Present
        SDL_BlitSurface(displaySurface, nullptr, windowSurface, nullptr);
        SDL_UpdateWindowSurface(window);

        // Init TTF
        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init: " << TTF_GetError() << std::endl;
        } else {
            // Try to load font
            // Prioritize Tahoma as requested
            font = TTF_OpenFont("/System/Library/Fonts/Hiragino Sans GB.ttc", 12);

            // font = TTF_OpenFont("fonts/Tahoma.ttf", 12);
            if (!font) {
                font = TTF_OpenFont("fonts/s60snr.ttf", 12);
            }
            if (!font) {
                font = TTF_OpenFont("/Library/Fonts/Arial.ttf", 12);
            }
            if (!font) {
                std::cerr << "TTF_OpenFont failed: " << TTF_GetError() << std::endl;
            }
        }
    }

    void setClip(int x, int y, int w, int h) {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        if (surface) {
            SDL_Rect rect = {x, y, w, h};
            SDL_SetClipRect(surface, &rect);
        }
    }

    void setColor(int r, int g, int b) {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        if (surface) {
            currentColor = SDL_MapRGB(surface->format, r, g, b);
            currentSDLColor = {(Uint8)r, (Uint8)g, (Uint8)b, 255};
        }
    }

    void drawLine(int x1, int y1, int x2, int y2) {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        // Bresenham's line algorithm
        int dx = abs(x2 - x1);
        int sx = x1 < x2 ? 1 : -1;
        int dy = -abs(y2 - y1);
        int sy = y1 < y2 ? 1 : -1;
        int err = dx + dy;
        
        while (true) {
            putPixel(x1, y1);
            if (x1 == x2 && y1 == y2) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x1 += sx; }
            if (e2 <= dx) { err += dx; y1 += sy; }
        }
    }

    void fillRect(int x, int y, int w, int h) {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        SDL_Rect rect = {x, y, w, h};
        SDL_FillRect(surface, &rect, currentColor);
    }
    
    void commit() {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        if (surface && displaySurface) {
            SDL_BlitSurface(surface, nullptr, displaySurface, nullptr);
        }
    }
    
    void update() { // Called by Main Loop (present)
        std::lock_guard<std::mutex> lock(surfaceMutex);
        if (window && windowSurface && displaySurface) {
            SDL_BlitSurface(displaySurface, nullptr, windowSurface, nullptr);
            SDL_UpdateWindowSurface(window);
        }
    }

    // New method for image drawing
    SDL_Surface* createImage(const unsigned char* data, int len) {
        // Use stb_image to decode
        int w, h, n;
        unsigned char* pixels = stbi_load_from_memory(data, len, &w, &h, &n, 4); // Force RGBA
        if (!pixels) return nullptr;
        
        // Create SDL surface
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
        if (surface) {
            SDL_LockSurface(surface);
            
            // Correct copy respecting pitch
            for (int y = 0; y < h; y++) {
                uint8_t* srcRow = pixels + (y * w * 4);
                uint8_t* dstRow = (uint8_t*)surface->pixels + (y * surface->pitch);
                memcpy(dstRow, srcRow, w * 4);
            }

            SDL_UnlockSurface(surface);
        }
        
        stbi_image_free(pixels);
        return surface;
    }

    void drawImage(SDL_Surface* img, int x, int y, int anchor) {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        if (!img || !surface) return;
        renderSurface(img, x, y, anchor, surface);
    }

    void drawImage(SDL_Surface* img, int x, int y, int anchor, SDL_Surface* dest) {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        if (!img || !dest) return;
        renderSurface(img, x, y, anchor, dest);
    }

    void drawString(const std::string& str, int x, int y, int anchor) {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        if (!font || !surface) return;
        
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, str.c_str(), currentSDLColor);
        if (textSurface) {
            renderSurface(textSurface, x, y, anchor, surface);
            SDL_FreeSurface(textSurface);
        }
    }

    void drawString(const std::string& str, int x, int y, int anchor, uint32_t colorRGB, SDL_Surface* target) {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        if (!font || !target) return;
        
        SDL_Color sdlColor = {(Uint8)((colorRGB >> 16) & 0xFF), (Uint8)((colorRGB >> 8) & 0xFF), (Uint8)(colorRGB & 0xFF), 255};
        
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, str.c_str(), sdlColor);
        if (textSurface) {
            renderSurface(textSurface, x, y, anchor, target);
            SDL_FreeSurface(textSurface);
        }
    }

private:
    void renderSurface(SDL_Surface* src, int x, int y, int anchor, SDL_Surface* destSurf) {
        int tx = x;
        int ty = y;
        
        // Handle Anchors (J2ME)
        // HCENTER = 1, VCENTER = 2, LEFT = 4, RIGHT = 8, TOP = 16, BOTTOM = 32, BASELINE = 64
        
        // Horizontal
        if (anchor & 1) { // HCENTER
            tx -= src->w / 2;
        } else if (anchor & 8) { // RIGHT
            tx -= src->w;
        }
        // Default is LEFT (no offset)
        
        // Vertical
        if (anchor & 32) { // BOTTOM
            ty -= src->h;
        } else if (anchor & 2) { // VCENTER
            ty -= src->h / 2;
        } else if (anchor & 64) { // BASELINE
            ty -= src->h; // Approximation
        }
        // Default is TOP (no offset)
        
        SDL_Rect dest = {tx, ty, 0, 0};
        SDL_BlitSurface(src, nullptr, destSurf, &dest);
    }

    GraphicsContext() = default;
    SDL_Window* window = nullptr;
    SDL_Surface* windowSurface = nullptr; // Actual Window Surface
    SDL_Surface* displaySurface = nullptr; // Front Buffer (Ready to show)
    SDL_Surface* surface = nullptr; // Back Buffer (Drawing Target)
    uint32_t currentColor = 0; 
    SDL_Color currentSDLColor = {0, 0, 0, 255};
    std::mutex surfaceMutex;
    TTF_Font* font = nullptr;

    void putPixel(int x, int y) {
        if (!surface) return;
        if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) return;
        
        // Use pitch-safe pixel access
        uint8_t* p = (uint8_t*)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
        
        // We need to copy bytes carefully based on format, but since we map currentColor using surface format:
        if (surface->format->BytesPerPixel == 4) {
            *(uint32_t*)p = currentColor;
        } else if (surface->format->BytesPerPixel == 2) {
             *(uint16_t*)p = (uint16_t)currentColor;
        } else {
             // Fallback for others
             // But we only support 32bit or 16bit really
             *(uint32_t*)p = currentColor;
        }
    }
};

} // namespace platform
} // namespace j2me
