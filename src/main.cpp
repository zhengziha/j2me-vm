#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <algorithm>
#include <string>
#include <vector>
#include <sys/stat.h>

#include "core/VMConfig.hpp"
#include "core/J2MEVM.hpp"
#include "core/Logger.hpp"
#include "platform/GraphicsContext.hpp"
#include "util/FileUtils.hpp"

// Helper to parse Manifest for MIDlet-1
std::string parseMidletClassName(const std::string& manifestContent) {
    std::string midlet1Line;
    size_t pos = 0;
    while (pos < manifestContent.length()) {
        size_t end = manifestContent.find('\n', pos);
        if (end == std::string::npos) end = manifestContent.length();
        std::string line = manifestContent.substr(pos, end - pos);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.rfind("MIDlet-1:", 0) == 0) {
            midlet1Line = line.substr(9);
            break;
        }
        pos = end + 1;
    }
    
    if (!midlet1Line.empty()) {
        size_t firstComma = midlet1Line.find(',');
        if (firstComma != std::string::npos) {
            size_t secondComma = midlet1Line.find(',', firstComma + 1);
            if (secondComma != std::string::npos) {
                std::string className = midlet1Line.substr(secondComma + 1);
                size_t start = className.find_first_not_of(" \t");
                if (start != std::string::npos) className = className.substr(start);
                size_t end = className.find_last_not_of(" \t");
                if (end != std::string::npos) className = className.substr(0, end + 1);
                std::replace(className.begin(), className.end(), '.', '/');
                return className + ".class";
            }
        }
    }
    return "";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: j2me-vm [--log-level LEVEL] <path_to_jar_or_class>" << std::endl;
        std::cout << "  LEVEL: debug, info, error, none (default: info)" << std::endl;
        return 1;
    }

    // Parse command line arguments
    j2me::core::VMConfig config;
    config.logLevel = j2me::core::LogLevel::INFO;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--log-level" && i + 1 < argc) {
            std::string levelStr = argv[++i];
            if (levelStr == "debug") {
                config.logLevel = j2me::core::LogLevel::DEBUG;
            } else if (levelStr == "info") {
                config.logLevel = j2me::core::LogLevel::INFO;
            } else if (levelStr == "error") {
                config.logLevel = j2me::core::LogLevel::ERROR;
            } else if (levelStr == "none") {
                config.logLevel = j2me::core::LogLevel::NONE;
            } else {
                std::cerr << "Invalid log level: " << levelStr << std::endl;
                return 1;
            }
        } else if (arg[0] != '-') {
            config.filePath = arg;
        }
    }

    if (config.filePath.empty()) {
        std::cerr << "Error: No file specified" << std::endl;
        return 1;
    }

    // Set log level global
    j2me::core::Logger::getInstance().setLevel(config.logLevel);

    // Initialize SDL (MUST BE ON MAIN THREAD)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        LOG_ERROR("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
    } else {
        LOG_INFO("SDL Initialized.");
    }

    // Create window
    // Initial size set to 3x scale (720x960) for better visibility on modern screens
    SDL_Window* window = SDL_CreateWindow("J2ME Emulator",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          240, 320, 
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == nullptr) {
        LOG_ERROR("Window could not be created! SDL_Error: " + std::string(SDL_GetError()));
        return 1;
    }
    LOG_INFO("Window Created.");
    
    // Init Graphics Context with Logical Resolution
    j2me::platform::GraphicsContext::getInstance().init(window, 240, 320);
    
    // Check if it's a .class file
    config.isClass = j2me::util::FileUtils::isClassFile(config.filePath);
    
    // Load Loaders
    config.appLoader = std::make_shared<j2me::loader::JarLoader>();
    config.libraryLoader = std::make_shared<j2me::loader::JarLoader>();
    
    // Load Library
    bool libraryLoaded = false;
    std::vector<std::string> libraryPaths;
    
    if (!config.filePath.empty()) {
        size_t lastSlash = config.filePath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            std::string jarDir = config.filePath.substr(0, lastSlash);
            libraryPaths.push_back(jarDir + "/stubs/rt.jar");
            libraryPaths.push_back(jarDir + "/../stubs/rt.jar");
        }
    }
    libraryPaths.push_back("/Users/zhengzihang/my-src/j2me-vm/stubs/rt.jar");
    libraryPaths.push_back("stubs/rt.jar");
    libraryPaths.push_back("../stubs/rt.jar");
    
    for (const auto& path : libraryPaths) {
        LOG_INFO("Trying library path: " + path);
        if (config.libraryLoader->load(path)) {
            LOG_INFO("Loaded library classes (rt.jar)");
            libraryLoaded = true;
            break;
        }
    }
    
    if (!libraryLoaded) {
        LOG_ERROR("Warning: rt.jar not found. Library classes might be missing.");
    }
    
    if (config.isClass) {
        config.classData = j2me::util::FileUtils::readFile(config.filePath);
        if (!config.classData) {
            LOG_ERROR("Failed to read .class file: " + config.filePath);
            return 1;
        }
        
        // Resolve class name
        std::string className = config.filePath;
        if (className.length() >= 6 && className.substr(className.length() - 6) == ".class") {
            className = className.substr(0, className.length() - 6);
        }
        size_t lastSep = className.find_last_of("/\\");
        if (lastSep != std::string::npos) {
            className = className.substr(lastSep + 1);
        }
        std::replace(className.begin(), className.end(), '.', '/');
        config.mainClassName = className;
        
    } else {
        // JAR Mode
        LOG_INFO("Loading JAR: " + config.filePath);
        if (!config.appLoader->load(config.filePath)) {
            LOG_ERROR("Failed to load JAR file.");
            return 1;
        }
        
        auto manifest = config.appLoader->getManifest();
        if (manifest) {
            LOG_INFO("Manifest found:\n" + *manifest);
            std::string midletClass = parseMidletClassName(*manifest);
            if (!midletClass.empty()) {
                config.mainClassName = midletClass;
                LOG_INFO("Found MIDlet-1 class: " + config.mainClassName);
            }
        } else {
             LOG_INFO("No Manifest found.");
        }
        
        if (config.mainClassName.empty()) {
            // Heuristic fallbacks
            config.mainClassName = "HelloWorld.class";
            if (!config.appLoader->hasFile(config.mainClassName)) config.mainClassName = "Point.class";
            if (!config.appLoader->hasFile(config.mainClassName)) config.mainClassName = "GraphicsTest.class";
            if (!config.appLoader->hasFile(config.mainClassName)) config.mainClassName = "InputTest.class";
            if (!config.appLoader->hasFile(config.mainClassName)) config.mainClassName = "ResourceTest.class";
            if (!config.appLoader->hasFile(config.mainClassName)) config.mainClassName = "RMSTest.class";
            if (!config.appLoader->hasFile(config.mainClassName)) config.mainClassName = "ImageTest.class";
        }
    }

    // Run VM
    j2me::core::J2MEVM vm;
    int result = vm.run(config);
    
    LOG_INFO("VM Stopped. Cleaning up SDL.");

    SDL_DestroyWindow(window);
    SDL_Quit();
    
    LOG_INFO("Shutdown complete.");
    return result;
}
