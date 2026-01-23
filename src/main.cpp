#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <algorithm> // for std::replace
#include <string>
#include <vector>
#include <sys/stat.h>
#include "loader/JarLoader.hpp"
#include "core/ClassParser.hpp"
#include "core/Interpreter.hpp"
#include "core/EventLoop.hpp"
#include "core/HeapManager.hpp"
#include "core/Logger.hpp"
#include "platform/GraphicsContext.hpp"
// #include "native/javax_microedition_lcdui_Graphics.cpp" // Don't include .cpp!
#include "native/javax_microedition_lcdui_Graphics.hpp" 
#include "native/javax_microedition_lcdui_Display.hpp"
#include "native/javax_microedition_lcdui_Image.hpp"
#include "native/javax_microedition_rms_RecordStore.hpp"
#include "native/java_lang_Class.hpp"
#include "native/java_lang_Object.hpp"
#include "native/java_lang_String.hpp"
#include "native/java_lang_StringBuffer.hpp"
#include "native/java_io_InputStream.hpp"
#include "native/java_io_PrintStream.hpp"
#include "native/java_lang_System.hpp"
#include "native/java_lang_Thread.hpp"
#include "native/java_util_Timer.hpp"
#include "core/TimerManager.hpp"
#include <fstream>
#include <optional>

// Helper function to check if a file is a .class file
bool isClassFile(const std::string& path) {
    return path.size() >= 6 && path.substr(path.size() - 6) == ".class";
}

// Helper function to read a .class file
std::optional<std::vector<uint8_t>> readClassFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(size);
    if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
        return std::nullopt;
    }
    
    return data;
}

// Helper function to check if a class extends MIDlet
bool isMIDletClass(std::shared_ptr<j2me::core::JavaClass> cls) {
    if (!cls) return false;
    
    // Walk up the class hierarchy
    auto current = cls;
    while (current) {
        if (current->name == "javax/microedition/midlet/MIDlet") {
            return true;
        }
        current = current->superClass;
    }
    return false;
}

// Helper function to check if a class has a main method
bool hasMainMethod(std::shared_ptr<j2me::core::JavaClass> cls) {
    if (!cls || !cls->rawFile) return false;
    
    for (const auto& method : cls->rawFile->methods) {
        auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(cls->rawFile->constant_pool[method.name_index]);
        auto desc = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(cls->rawFile->constant_pool[method.descriptor_index]);
        
        if (name && desc && name->bytes == "main" && desc->bytes == "([Ljava/lang/String;)V") {
            return true;
        }
    }
    return false;
}

struct VMConfig {
    std::string filePath;
    j2me::core::LogLevel logLevel;
    std::shared_ptr<j2me::loader::JarLoader> appLoader;
    std::shared_ptr<j2me::loader::JarLoader> libraryLoader;
    std::string mainClassName;
    bool isClass; // true if loading single class file
    std::optional<std::vector<uint8_t>> classData; // if single class
};

void runVM(VMConfig config) {
    // Set log level
    j2me::core::Logger::getInstance().setLevel(config.logLevel);

    // Register natives
    j2me::natives::registerGraphicsNatives();
    j2me::natives::registerDisplayNatives();
    j2me::natives::registerImageNatives();
    j2me::natives::registerRecordStoreNatives();
    j2me::natives::registerClassNatives();
    j2me::natives::registerObjectNatives();
    j2me::natives::registerStringNatives();
    j2me::natives::registerStringBufferNatives();
    j2me::natives::registerPrintStreamNatives();
    j2me::natives::registerSystemNatives();
    j2me::natives::registerInputStreamNatives();
    j2me::natives::registerThreadNatives();
    j2me::natives::registerTimerNatives();

    LOG_INFO("Starting J2ME VM Thread...");

    // Create interpreter
    // If isClass, use libraryLoader as base, else appLoader
    j2me::loader::JarLoader& baseLoader = config.isClass ? *config.libraryLoader : *config.appLoader;
    j2me::core::Interpreter interpreter(baseLoader);
    interpreter.setLibraryLoader(config.libraryLoader);
    j2me::core::NativeRegistry::getInstance().setInterpreter(&interpreter);
    if (!config.isClass && config.appLoader) {
        j2me::core::NativeRegistry::getInstance().setJarLoader(config.appLoader.get());
    }

    std::shared_ptr<j2me::core::JavaObject> midletInstance;

    if (config.isClass) {
        // Handle .class file directly
        LOG_INFO("Loading .class file: " + config.filePath);
        
        if (!config.classData) {
            LOG_ERROR("Class data missing for .class file");
            return;
        }
        
        // Parse the class file
        j2me::core::ClassParser parser;
        auto classFile = parser.parse(*config.classData);
        LOG_INFO("Successfully parsed class file!");
        
        // Get class name from file path or config
        std::string className = config.mainClassName;
        
        // Create JavaClass from parsed class file
        auto javaClass = std::make_shared<j2me::core::JavaClass>(classFile);
        
        // Link the class
        if (classFile->super_class != 0) {
            auto superInfo = std::dynamic_pointer_cast<j2me::core::ConstantClass>(classFile->constant_pool[classFile->super_class]);
            auto superNameInfo = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(classFile->constant_pool[superInfo->name_index]);
            std::string superName = superNameInfo->bytes;
            
            if (superName != "java/lang/Object") {
                auto superClass = interpreter.resolveClass(superName);
                javaClass->link(superClass);
            } else {
                javaClass->link(nullptr);
            }
        } else {
            javaClass->link(nullptr);
        }
        
        // Register the class with the interpreter
        interpreter.registerClass(className, javaClass);
        LOG_INFO("Registered class: " + className);
        
        // Check if it's a MIDlet or has main method
        bool isMIDlet = isMIDletClass(javaClass);
        bool hasMain = hasMainMethod(javaClass);
        
        // If it has main method and is not a MIDlet, run main and exit
        if (hasMain && !isMIDlet) {
            LOG_INFO("Running main method (headless mode)...");
            
            // Find and execute main method
            for (const auto& method : javaClass->rawFile->methods) {
                auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(javaClass->rawFile->constant_pool[method.name_index]);
                auto desc = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(javaClass->rawFile->constant_pool[method.descriptor_index]);
                
                if (name && desc && name->bytes == "main" && desc->bytes == "([Ljava/lang/String;)V") {
                    auto frame = std::make_shared<j2me::core::StackFrame>(method, javaClass->rawFile);
                    
                    // Create String array for args (empty for now)
                    auto arrayCls = interpreter.resolveClass("[Ljava/lang/String;");
                    if (arrayCls) {
                        auto arrayObj = j2me::core::HeapManager::getInstance().allocate(arrayCls);
                        arrayObj->fields.resize(0); // Empty array
                        
                        j2me::core::JavaValue vArgs;
                        vArgs.type = j2me::core::JavaValue::REFERENCE;
                        vArgs.val.ref = arrayObj;
                        frame->setLocal(0, vArgs);
                    }
                    
                    interpreter.execute(frame);
                    LOG_INFO("main method completed.");
                    return;
                }
            }
        }
        
        if (isMIDlet) {
             LOG_INFO("Running MIDlet (GUI mode)...");
             // Instantiate and prepare for loop
             // For single class MIDlet, we need to alloc and init
             auto cls = interpreter.resolveClass(className);
             midletInstance.reset(j2me::core::HeapManager::getInstance().allocate(cls));
             
             // Call <init>
             for (const auto& method : classFile->methods) {
                auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(classFile->constant_pool[method.name_index]);
                if (name->bytes == "<init>") {
                     auto frame = std::make_shared<j2me::core::StackFrame>(method, classFile);
                     j2me::core::JavaValue vThis; vThis.type = j2me::core::JavaValue::REFERENCE; vThis.val.ref = midletInstance.get();
                     frame->setLocal(0, vThis);
                     interpreter.execute(frame);
                     break;
                }
            }
            
            // Call startApp
            // ... (similar to below, but for single class)
             // Check inheritance (simplified)
            auto currentCls = cls;
            bool startAppFound = false;
            while (currentCls) {
                for (const auto& method : currentCls->rawFile->methods) {
                    auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(currentCls->rawFile->constant_pool[method.name_index]);
                    if (name->bytes == "startApp") {
                            auto frame = std::make_shared<j2me::core::StackFrame>(method, currentCls->rawFile);
                            j2me::core::JavaValue vThis; vThis.type = j2me::core::JavaValue::REFERENCE; vThis.val.ref = midletInstance.get();
                            frame->setLocal(0, vThis);
                            interpreter.execute(frame);
                            startAppFound = true;
                            break;
                    }
                }
                if (startAppFound) break;
                currentCls = currentCls->superClass;
            }
        } else {
            LOG_ERROR("Class is not a MIDlet and has no main method. Cannot run.");
            return;
        }

    } else {
        // JAR Mode
        if (config.appLoader->hasFile(config.mainClassName)) {
            auto classData = config.appLoader->getFile(config.mainClassName);
            if (classData) {
                LOG_INFO("Found " + config.mainClassName + ", parsing...");
                try {
                    j2me::core::ClassParser parser;
                    auto classFile = parser.parse(*classData);
                    LOG_INFO("Successfully parsed class file!");
                    
                    // 1. Create instance (ALLOC)
                    LOG_INFO("Resolving main class...");
                    // remove .class if present
                    std::string clsName = config.mainClassName;
                    if (clsName.size() >= 6 && clsName.substr(clsName.size()-6) == ".class") 
                        clsName = clsName.substr(0, clsName.size()-6);
                        
                    auto cls = interpreter.resolveClass(clsName); 
                    LOG_INFO("Allocating instance..." + clsName);
                    midletInstance.reset(j2me::core::HeapManager::getInstance().allocate(cls));

                    // 2. Call constructor (init)
                    LOG_INFO("Finding <init>..." + clsName);
                    for (const auto& method : classFile->methods) {
                        auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(classFile->constant_pool[method.name_index]);
                        if (name->bytes == "<init>") {
                             LOG_INFO("Executing <init>..." + clsName);
                             auto frame = std::make_shared<j2me::core::StackFrame>(method, classFile);
                             j2me::core::JavaValue vThis; vThis.type = j2me::core::JavaValue::REFERENCE; vThis.val.ref = midletInstance.get();
                             frame->setLocal(0, vThis);
                             interpreter.execute(frame);
                             break;
                        }
                    }
                    LOG_INFO("<init> done." + clsName);
                    
                    // 3. Call startApp()
                    LOG_INFO("Looking for startApp method...");
                    auto currentCls = cls;
                    bool startAppFound = false;
                    
                    while (currentCls) {
                        for (const auto& method : currentCls->rawFile->methods) {
                            auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(currentCls->rawFile->constant_pool[method.name_index]);
                            if (name->bytes == "startApp") {
                                 LOG_INFO("Calling startApp() in class " + currentCls->name);
                                 auto frame = std::make_shared<j2me::core::StackFrame>(method, currentCls->rawFile);
                                 j2me::core::JavaValue vThis; vThis.type = j2me::core::JavaValue::REFERENCE; vThis.val.ref = midletInstance.get();
                                 frame->setLocal(0, vThis);
                                 interpreter.execute(frame);
                                 LOG_INFO("startApp() done.");
                                 startAppFound = true;
                                 break;
                           }
                        }
                        if (startAppFound) break;
                        currentCls = currentCls->superClass;
                    }
                    
                    if (!startAppFound) {
                        LOG_INFO("startApp not found in hierarchy.");
                    }
                    
                } catch (const std::exception& e) {
                    LOG_ERROR("Exception during VM execution: " + std::string(e.what()));
                    std::exit(1);
                }
            }
        }
    }
    
    // Enter VM Loop (handle events from queue, render)
    LOG_INFO("Entering VM Event Loop...");
    j2me::core::EventLoop& eventLoop = j2me::core::EventLoop::getInstance();
    while (!eventLoop.shouldExit()) {
        try {
            eventLoop.dispatchEvents(&interpreter);
            j2me::core::TimerManager::getInstance().tick(&interpreter);
            eventLoop.render(&interpreter);
        } catch (const std::exception& e) {
             LOG_ERROR("Exception in VM Loop: " + std::string(e.what()));
             std::exit(1);
        }
        
        // Small sleep to yield CPU and prevent 100% usage on VM thread
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    LOG_INFO("VM Thread Exiting...");
    std::exit(0);
}

int main(int argc, char* argv[]) {
    // Auto-exit after 120 seconds
    std::thread([]() {
        std::this_thread::sleep_for(std::chrono::seconds(120));
        std::cout << "\n[System] Timeout reached (120s). Exiting process.\n" << std::endl;
        std::exit(0);
    }).detach();

    if (argc < 2) {
        std::cout << "Usage: j2me-vm [--log-level LEVEL] <path_to_jar_or_class>" << std::endl;
        std::cout << "  LEVEL: debug, info, error, none (default: info)" << std::endl;
        return 1;
    }

    // Parse command line arguments
    VMConfig config;
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

    // Check if it's a .class file
    config.isClass = isClassFile(config.filePath);
    
    // Set log level global
    j2me::core::Logger::getInstance().setLevel(config.logLevel);

    // Initialize SDL (MUST BE ON MAIN THREAD)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        LOG_ERROR("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
    } else {
        LOG_INFO("SDL Initialized.");
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("J2ME Emulator",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          240, 320, // Standard J2ME resolution
                                          SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        LOG_ERROR("Window could not be created! SDL_Error: " + std::string(SDL_GetError()));
        return 1;
    }
    LOG_INFO("Window Created.");
    
    // Init Graphics Context
    j2me::platform::GraphicsContext::getInstance().init(window);
    
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
        config.classData = readClassFile(config.filePath);
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
            // Parse MIDlet-1
            std::string content = *manifest;
            std::string midlet1Line;
            size_t pos = 0;
            while (pos < content.length()) {
                size_t end = content.find('\n', pos);
                if (end == std::string::npos) end = content.length();
                std::string line = content.substr(pos, end - pos);
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
                         config.mainClassName = className + ".class";
                         LOG_INFO("Found MIDlet-1 class: " + config.mainClassName);
                     }
                 }
            }
        } else {
             LOG_INFO("No Manifest found.");
        }
        
        if (config.mainClassName.empty()) {
            config.mainClassName = "HelloWorld.class";
            // Fallback logic
            if (!config.appLoader->hasFile(config.mainClassName)) config.mainClassName = "Point.class";
            if (!config.appLoader->hasFile(config.mainClassName)) config.mainClassName = "GraphicsTest.class";
            if (!config.appLoader->hasFile(config.mainClassName)) config.mainClassName = "InputTest.class";
            if (!config.appLoader->hasFile(config.mainClassName)) config.mainClassName = "ResourceTest.class";
            if (!config.appLoader->hasFile(config.mainClassName)) config.mainClassName = "RMSTest.class";
            if (!config.appLoader->hasFile(config.mainClassName)) config.mainClassName = "ImageTest.class";
        }
    }

    // Launch VM Thread
    LOG_INFO("Launching VM Thread...");
    std::thread vmThread(runVM, config);
    vmThread.detach();

    // Main Thread Event Loop
    LOG_INFO("Entering Main Event Loop (SDL)...");
    j2me::core::EventLoop& eventLoop = j2me::core::EventLoop::getInstance();
    while (!eventLoop.shouldExit()) {
        eventLoop.pollSDL();
        // Update screen from backbuffer
        j2me::platform::GraphicsContext::getInstance().update();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
    
    LOG_INFO("Main Thread Exiting...");
    return 0;
}
