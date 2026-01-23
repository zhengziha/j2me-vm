#pragma once

#include <SDL2/SDL.h>
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
        // J2ME uses a software buffer usually, but we can draw to surface directly or renderer
        // For simplicity, let's use SDL_Surface of the window
        surface = SDL_GetWindowSurface(window);
        
        // Fill white
        SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 255, 255));
        SDL_UpdateWindowSurface(window);
    }

    void setColor(int r, int g, int b) {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        if (surface) {
            currentColor = SDL_MapRGB(surface->format, r, g, b);
        }
    }

    void drawLine(int x1, int y1, int x2, int y2) {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        // SDL2 doesn't have drawLine on Surface easily without Renderer, 
        // but for Phase 3 prototype we can use a simple Bresenham or just direct pixel if horizontal/vertical.
        // Actually, let's use SDL_Renderer if possible, but our main loop used Surface.
        // Let's stick to Surface and write a naive line drawer or use SDL_FillRect for lines if they are straight.
        // Or better: switch to Renderer in main.cpp? 
        // For now, let's implement a really bad line drawer for 1px dots or just rects.
        
        // Let's just implement fillRect for now which is easier on Surface.
        // If line is needed, we'll draw dots.
        
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
        // update(); // Removed to prevent flickering
    }

    void fillRect(int x, int y, int w, int h) {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        SDL_Rect rect = {x, y, w, h};
        SDL_FillRect(surface, &rect, currentColor);
        // update(); // Removed to prevent flickering
    }
    
    void update() {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        SDL_UpdateWindowSurface(window);
    }

    // New method for image drawing
    SDL_Surface* createImage(const unsigned char* data, int len) {
        // Use stb_image to decode
        int w, h, n;
        unsigned char* pixels = stbi_load_from_memory(data, len, &w, &h, &n, 4); // Force RGBA
        if (!pixels) return nullptr;
        
        // Create SDL surface
        // SDL_CreateRGBSurfaceFrom uses the pixels buffer directly, so we must not free it until surface is freed.
        // But SDL surfaces are tricky with external buffers.
        // Better to create a new surface and blit, or use SDL_CreateRGBSurfaceWithFormatFrom.
        
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
        if (surface) {
            SDL_LockSurface(surface);
            memcpy(surface->pixels, pixels, w * h * 4);
            SDL_UnlockSurface(surface);
        }
        
        stbi_image_free(pixels);
        return surface;
    }

    void drawImage(SDL_Surface* img, int x, int y) {
        std::lock_guard<std::mutex> lock(surfaceMutex);
        if (!img || !surface) return;
        SDL_Rect dest = {x, y, 0, 0};
        SDL_BlitSurface(img, nullptr, surface, &dest);
        // update(); // Removed to prevent flickering
    }

private:
    GraphicsContext() = default;
    SDL_Window* window = nullptr;
    SDL_Surface* surface = nullptr;
    uint32_t currentColor = 0; // Black by default (0) or whatever
    std::mutex surfaceMutex;

    void putPixel(int x, int y) {
        if (!surface) return;
        if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) return;
        
        // Assuming 32-bit for simplicity on modern systems
        uint32_t* pixels = (uint32_t*)surface->pixels;
        pixels[y * surface->w + x] = currentColor;
    }
};

} // namespace platform
} // namespace j2me
