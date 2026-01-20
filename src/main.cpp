#include <iostream>
#include <SDL2/SDL.h>
#include <algorithm> // for std::replace
#include <string>
#include <vector>
#include <sys/stat.h>
#include "loader/JarLoader.hpp"
#include "core/ClassParser.hpp"
#include "core/Interpreter.hpp"
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
#include "native/java_io_InputStream.hpp"
#include "native/java_io_PrintStream.hpp"
#include "native/java_lang_System.hpp"
#include "native/java_lang_Thread.hpp"
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: j2me-vm [--log-level LEVEL] <path_to_jar_or_class>" << std::endl;
        std::cout << "  LEVEL: debug, info, error, none (default: info)" << std::endl;
        return 1;
    }

    // Parse command line arguments
    std::string filePath;
    j2me::core::LogLevel logLevel = j2me::core::LogLevel::INFO;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--log-level" && i + 1 < argc) {
            std::string levelStr = argv[++i];
            if (levelStr == "debug") {
                logLevel = j2me::core::LogLevel::DEBUG;
            } else if (levelStr == "info") {
                logLevel = j2me::core::LogLevel::INFO;
            } else if (levelStr == "error") {
                logLevel = j2me::core::LogLevel::ERROR;
            } else if (levelStr == "none") {
                logLevel = j2me::core::LogLevel::NONE;
            } else {
                std::cerr << "Invalid log level: " << levelStr << std::endl;
                return 1;
            }
        } else if (arg[0] != '-') {
            filePath = arg;
        }
    }

    if (filePath.empty()) {
        std::cerr << "Error: No file specified" << std::endl;
        return 1;
    }

    // Check if it's a .class file
    bool isClass = isClassFile(filePath);
    
    // Set log level
    j2me::core::Logger::getInstance().setLevel(logLevel);

    // Register natives
    j2me::natives::registerGraphicsNatives();
    j2me::natives::registerDisplayNatives();
    j2me::natives::registerImageNatives();
    j2me::natives::registerRecordStoreNatives();
    j2me::natives::registerClassNatives();
    j2me::natives::registerObjectNatives();
    j2me::natives::registerStringNatives();
    j2me::natives::registerPrintStreamNatives();
    j2me::natives::registerSystemNatives();
    j2me::natives::registerInputStreamNatives();
    j2me::natives::registerThreadNatives();

    LOG_INFO("Starting J2ME VM...");

    // Handle .class file directly
    if (isClass) {
        LOG_INFO("Loading .class file: " + filePath);
        
        auto classData = readClassFile(filePath);
        if (!classData) {
            LOG_ERROR("Failed to read .class file: " + filePath);
            return 1;
        }
        
        // Parse the class file
        j2me::core::ClassParser parser;
        auto classFile = parser.parse(*classData);
        LOG_INFO("Successfully parsed class file!");
        
        // Create a minimal JarLoader for library classes
        auto libraryLoader = std::make_shared<j2me::loader::JarLoader>();
        bool libraryLoaded = false;
        
        // Try to load rt.jar
        std::vector<std::string> libraryPaths;
        
        // Find directory of the .class file
        size_t lastSlash = filePath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            std::string classDir = filePath.substr(0, lastSlash);
            // libraryPaths.push_back(classDir + "/stubs/rt.jar");
            // libraryPaths.push_back(classDir + "/../stubs/rt.jar");
        }
        
        libraryPaths.push_back("/Users/zhengzihang/my-src/j2me-vm/stubs/rt.jar");
        libraryPaths.push_back("stubs/rt.jar");
        libraryPaths.push_back("../stubs/rt.jar");
        
        for (const auto& path : libraryPaths) {
            LOG_DEBUG("Trying library path: " + path);
            if (libraryLoader->load(path)) {
                LOG_INFO("Loaded library classes (rt.jar)");
                libraryLoaded = true;
                break;
            }
        }
        
        if (!libraryLoaded) {
            LOG_ERROR("Warning: rt.jar not found. Library classes might be missing.");
        }
        
        // Create interpreter
        j2me::core::Interpreter interpreter(*libraryLoader);
        interpreter.setLibraryLoader(libraryLoader);
        j2me::core::NativeRegistry::getInstance().setInterpreter(&interpreter);
        
        // Get class name from file path
        std::string className = filePath;
        
        LOG_DEBUG("Original file path: " + className);
        
        // Remove .class extension first
        if (className.length() >= 6 && className.substr(className.length() - 6) == ".class") {
            className = className.substr(0, className.length() - 6);
        }
        
        LOG_DEBUG("After removing .class: " + className);
        
        // Find the last directory separator
        size_t lastSep = className.find_last_of("/\\");
        if (lastSep != std::string::npos) {
            // Extract part after last separator
            className = className.substr(lastSep + 1);
        }
        
        LOG_DEBUG("After extracting filename: " + className);
        
        // Replace . with / for package names
        std::replace(className.begin(), className.end(), '.', '/');
        
        LOG_DEBUG("Resolved class name: " + className);
        
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
        
        LOG_INFO("Class: " + className);
        LOG_INFO("Is MIDlet: " + std::string(isMIDlet ? "yes" : "no"));
        LOG_INFO("Has main method: " + std::string(hasMain ? "yes" : "no"));
        
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
                    return 0;
                }
            }
            
            LOG_ERROR("main method not found!");
            return 1;
        }
        
        // If it's a MIDlet, we need to run it with GUI
        if (isMIDlet) {
            LOG_INFO("Running MIDlet (GUI mode)...");
            // Fall through to the GUI code below
        } else {
            LOG_ERROR("Class is not a MIDlet and has no main method. Cannot run.");
            return 1;
        }
    }
    
    // For .jar files or MIDlets, continue with the existing GUI code
    LOG_INFO("Loading JAR: " + filePath);

    j2me::loader::JarLoader loader;
    if (!loader.load(filePath)) {
        LOG_ERROR("Failed to load JAR file.");
        return 1;
    }
    j2me::core::NativeRegistry::getInstance().setJarLoader(&loader);

    auto manifest = loader.getManifest();
    if (manifest) {
        LOG_INFO("Manifest found:\n" + *manifest);
    } else {
        LOG_INFO("No Manifest found.");
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        // Just print error and continue if headless, but we really need it for InputTest
        LOG_ERROR("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
        // return 1; // Don't return, maybe we can run in headless mode? 
        // But InputTest needs events.
        // In the sandbox, maybe we have a dummy driver?
        // Let's assume we can proceed or mock it if fails.
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
    
    // Init Graphics Context (even if window is null, it might handle it safely?)
    j2me::platform::GraphicsContext::getInstance().init(window);

    // Load Main Class
    std::string mainClassName = "HelloWorld.class";

    if (manifest) {
        // Parse Manifest string line by line
        std::string content = *manifest;
        std::string midlet1Line;
        
        size_t pos = 0;
        while (pos < content.length()) {
            size_t end = content.find('\n', pos);
            if (end == std::string::npos) end = content.length();
            
            std::string line = content.substr(pos, end - pos);
            // Remove \r if present
            if (!line.empty() && line.back() == '\r') line.pop_back();
            
            if (line.rfind("MIDlet-1:", 0) == 0) { // Starts with "MIDlet-1:"
                midlet1Line = line.substr(9); // Skip "MIDlet-1:"
                // Handle continuation lines? J2ME Manifests can wrap lines.
                // But usually MIDlet-1 fits or we just take what we have for now.
                break;
            }
            
            pos = end + 1;
        }

        if (!midlet1Line.empty()) {
             // Parse MIDlet-1: Name, Icon, Class
             // e.g. "仙剑奇侠传, /sword.png, pal.GameMIDlet"
             size_t firstComma = midlet1Line.find(',');
             if (firstComma != std::string::npos) {
                 size_t secondComma = midlet1Line.find(',', firstComma + 1);
                 if (secondComma != std::string::npos) {
                     std::string className = midlet1Line.substr(secondComma + 1);
                     // Trim spaces
                     size_t start = className.find_first_not_of(" \t");
                     if (start != std::string::npos) {
                         className = className.substr(start);
                         size_t end = className.find_last_not_of(" \t");
                         if (end != std::string::npos) {
                             className = className.substr(0, end + 1);
                         }
                     }
                     
                     // Replace . with /
                     std::replace(className.begin(), className.end(), '.', '/');
                     
                     mainClassName = className + ".class";
                     LOG_INFO("Found MIDlet-1 class: " + mainClassName);
                 }
             }
        }
    }
    
    // Fallback if not found in Manifest or Manifest missing
    if (!loader.hasFile(mainClassName)) {
        // Fallback logic
        if (!loader.hasFile(mainClassName)) {
            mainClassName = "Point.class";
        }
        if (!loader.hasFile(mainClassName)) {
            mainClassName = "GraphicsTest.class";
        }
        if (!loader.hasFile(mainClassName)) {
            mainClassName = "InputTest.class"; // New input test
        }
        if (!loader.hasFile(mainClassName)) {
            mainClassName = "ResourceTest.class";
        }
        if (!loader.hasFile(mainClassName)) {
            mainClassName = "RMSTest.class";
        }
        if (!loader.hasFile(mainClassName)) {
            mainClassName = "ImageTest.class";
        }
    }

    // Load library classes (rt.jar)
    auto libraryLoader = std::make_shared<j2me::loader::JarLoader>();
    bool libraryLoaded = false;
    
    // Try multiple possible paths for rt.jar
    std::vector<std::string> libraryPaths;
    
    // First, try to find rt.jar relative to the JAR file
    if (!filePath.empty()) {
        // Find the last '/' or '\' to get the directory
        size_t lastSlash = filePath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            std::string jarDir = filePath.substr(0, lastSlash);
            
            // Try stubs/rt.jar relative to JAR directory
            std::string path1 = jarDir + "/stubs/rt.jar";
            libraryPaths.push_back(path1);
            LOG_DEBUG("Trying library path (relative to JAR): " + path1);
            
            // Try ../stubs/rt.jar relative to JAR directory
            std::string path2 = jarDir + "/../stubs/rt.jar";
            libraryPaths.push_back(path2);
            LOG_DEBUG("Trying library path (relative to JAR parent): " + path2);
        }
    }
    
    // Try absolute path
    std::string absPath = "/Users/zhengzihang/my-src/j2me-vm/stubs/rt.jar";
    libraryPaths.push_back(absPath);
    LOG_DEBUG("Trying library path (absolute): " + absPath);
    
    // Try relative paths
    libraryPaths.push_back("stubs/rt.jar");
    libraryPaths.push_back("../stubs/rt.jar");
    
    for (const auto& path : libraryPaths) {
        LOG_INFO("Trying library path: " + path);
        if (libraryLoader->load(path)) {
            LOG_INFO("Loaded library classes (rt.jar)");
            libraryLoaded = true;
            break;
        } else {
            LOG_INFO("Failed to load library from: " + path);
        }
    }
    
    if (!libraryLoaded) {
        LOG_ERROR("Warning: rt.jar not found. Library classes might be missing.");
    }

    std::shared_ptr<j2me::core::JavaObject> midletInstance;
    j2me::core::Interpreter interpreter(loader);
    interpreter.setLibraryLoader(libraryLoader);
    j2me::core::NativeRegistry::getInstance().setInterpreter(&interpreter);

    if (loader.hasFile(mainClassName)) {
        auto classData = loader.getFile(mainClassName);
        if (classData) {
            LOG_INFO("Found " + mainClassName + ", parsing...");
            try {
                j2me::core::ClassParser parser;
                auto classFile = parser.parse(*classData);
                LOG_INFO("Successfully parsed class file!");
                
                // For GraphicsTest, we want to create instance and call paint(Graphics)
                // 1. Create instance (ALLOC)
                LOG_INFO("Resolving main class...");
                auto cls = interpreter.resolveClass(mainClassName.substr(0, mainClassName.length()-6)); // remove .class
                LOG_INFO("Allocating instance..." + mainClassName);
                midletInstance.reset(j2me::core::HeapManager::getInstance().allocate(cls));

                // 2. Call constructor (init) if needed
                // We assume 0-arg constructor for GraphicsTest
                // We need to find <init> method
                LOG_INFO("Finding <init>..." + mainClassName);
                for (const auto& method : classFile->methods) {
                    auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(classFile->constant_pool[method.name_index]);
                    if (name->bytes == "<init>") {
                         LOG_INFO("Executing <init>..." + mainClassName);
                         auto frame = std::make_shared<j2me::core::StackFrame>(method, classFile);
                         // Set 'this'
                         j2me::core::JavaValue vThis; vThis.type = j2me::core::JavaValue::REFERENCE; vThis.val.ref = midletInstance.get();
                         frame->setLocal(0, vThis);
                         interpreter.execute(frame);
                         break;
                    }
                }
                LOG_INFO("<init> done." + mainClassName);
                
                // 3. Call startApp() if it is a MIDlet
                // Check inheritance (simplified)
                // Assuming it is a MIDlet for now if it has startApp
                LOG_INFO("Looking for startApp method...");
                for (const auto& method : cls->rawFile->methods) {
                    auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(cls->rawFile->constant_pool[method.name_index]);
                    auto desc = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(cls->rawFile->constant_pool[method.descriptor_index]);
                    LOG_DEBUG("Found method: " + name->bytes + desc->bytes);
                    if (name->bytes == "startApp") {
                         LOG_INFO("Calling startApp()...");
                         LOG_DEBUG("Method has " + std::to_string(method.attributes.size()) + " attributes");
                         for (const auto& attr : method.attributes) {
                             auto attrName = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(cls->rawFile->constant_pool[attr.attribute_name_index]);
                             LOG_DEBUG("  Attribute: " + attrName->bytes);
                         }
                        auto frame = std::make_shared<j2me::core::StackFrame>(method, cls->rawFile);
                        j2me::core::JavaValue vThis; vThis.type = j2me::core::JavaValue::REFERENCE; vThis.val.ref = midletInstance.get();
                        frame->setLocal(0, vThis);
                        interpreter.execute(frame);
                        LOG_INFO("startApp() done.");
                        break;
                   }
               }
               
               // Let's run 'main' if it exists (for console tests)
               // Or if it is a Canvas, we'll drive it in the loop.
               
               bool hasMain = false;
               for (const auto& method : cls->rawFile->methods) {
                   auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(cls->rawFile->constant_pool[method.name_index]);
                   if (name->bytes == "main") {
                        auto frame = std::make_shared<j2me::core::StackFrame>(method, cls->rawFile);
                        interpreter.execute(frame);
                        hasMain = true;
                        break;
                   }
               }
               
               if (!hasMain) {
                    LOG_INFO("No main method, assuming Canvas/MIDlet...");
               }

            } catch (const std::exception& e) {
                LOG_ERROR("Failed to run class file: " + std::string(e.what()));
                return 1;
            }
        }
    } else {
        LOG_ERROR(mainClassName + " not found in JAR");
    }

    // Main loop flag
    bool quit = false;
    SDL_Event e;

    // Key mapping helper
    auto mapKey = [](SDL_Keycode key) -> int {
        switch (key) {
            case SDLK_UP: return 1; // UP
            case SDLK_DOWN: return 6; // DOWN
            case SDLK_LEFT: return 2; // LEFT
            case SDLK_RIGHT: return 5; // RIGHT
            case SDLK_RETURN: return 8; // FIRE
            case SDLK_0: return 48;
            case SDLK_1: return 49;
            case SDLK_2: return 50;
            case SDLK_3: return 51;
            case SDLK_4: return 52;
            case SDLK_5: return 53;
            case SDLK_6: return 54;
            case SDLK_7: return 55;
            case SDLK_8: return 56;
            case SDLK_9: return 57;
            // case SDLK_8: return 42; // STAR (Duplicate case label)
            // Just basic mapping
            default: return 0;
        }
    };

    // While application is running
    while (!quit) {
        try {
            // Handle events on queue
            while (SDL_PollEvent(&e) != 0) {
                // User requests quit
                if (e.type == SDL_QUIT) {
                    quit = true;
                } else if (e.type == SDL_KEYDOWN) {
                     int keyCode = mapKey(e.key.keysym.sym);
                     if (keyCode != 0 && midletInstance) {
                         // Call keyPressed(keyCode)
                         auto cls = midletInstance->cls;
                         for (const auto& method : cls->rawFile->methods) {
                             auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(cls->rawFile->constant_pool[method.name_index]);
                             if (name->bytes == "keyPressed") {
                                 auto frame = std::make_shared<j2me::core::StackFrame>(method, cls->rawFile);
                                 
                                 // Push 'this'
                                 j2me::core::JavaValue vThis; vThis.type = j2me::core::JavaValue::REFERENCE; vThis.val.ref = midletInstance.get();
                                 frame->setLocal(0, vThis);
                                 
                                 // Push keyCode
                                 j2me::core::JavaValue vKey; vKey.type = j2me::core::JavaValue::INT; vKey.val.i = keyCode;
                                 frame->setLocal(1, vKey);
                                 
                                 interpreter.execute(frame);
                                 break;
                             }
                         }
                     }
                }
            }
            
            // If we have an instance (Canvas), call paint(Graphics)
            // Use getCurrentDisplayable() instead of midletInstance
            j2me::core::JavaObject* displayable = j2me::natives::getCurrentDisplayable();
            
            if (displayable) {
                 // Create a Graphics object
                 // For Phase 3, we just create a dummy JavaObject for Graphics
                 // We need to load Graphics class first?
                 // Let's cheat and pass null as Graphics for now? 
                 // No, the native methods pop 'this' (Graphics).
                 // So we need a Graphics instance.
                 
                 static std::shared_ptr<j2me::core::JavaClass> graphicsCls;
                 if (!graphicsCls) {
                     graphicsCls = interpreter.resolveClass("javax/microedition/lcdui/Graphics");
                 }
                 
                 if (graphicsCls) {
                     j2me::core::JavaObject* g = j2me::core::HeapManager::getInstance().allocate(graphicsCls);
                     
                     // Find paint method
                     auto cls = displayable->cls;
                     // Need to walk up hierarchy if not found!
                     // GraphicsTest extends Canvas. paint is in GraphicsTest.
                     // So simple lookup should work.
                     
                     bool found = false;
                     // Walk up the class hierarchy to find paint
                     auto currentCls = cls;
                     while (currentCls) {
                         for (const auto& method : currentCls->rawFile->methods) {
                             auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(currentCls->rawFile->constant_pool[method.name_index]);
                             if (name->bytes == "paint") {
                                 auto frame = std::make_shared<j2me::core::StackFrame>(method, currentCls->rawFile);
                                 
                                 // Push 'this' (Canvas)
                                 j2me::core::JavaValue vThis; vThis.type = j2me::core::JavaValue::REFERENCE; vThis.val.ref = displayable;
                                 frame->setLocal(0, vThis);
                                 
                                 // Push 'g' (Graphics)
                                 j2me::core::JavaValue vG; vG.type = j2me::core::JavaValue::REFERENCE; vG.val.ref = g;
                                 frame->setLocal(1, vG);
                                 
                                 interpreter.execute(frame);
                                 found = true;
                                 break;
                             }
                     }
                     if (found) break;
                     currentCls = currentCls->superClass;
                 }

                 if (!found) {
                     static bool warned = false;
                     if (!warned) {
                         LOG_ERROR("Warning: paint method not found in " + cls->name);
                         warned = true;
                     }
                 }
             } else {
                 // std::cerr << "Could not load Graphics class!" << std::endl;
             }
            }
            
            // SDL Update happens inside GraphicsContext natives, but we can do it here too
            j2me::platform::GraphicsContext::getInstance().update();
            SDL_Delay(16); // ~60FPS
        } catch (const std::exception& e) {
            LOG_ERROR("Runtime error in main loop: " + std::string(e.what()));
            quit = true;
        }
    }

    // Destroy window
    SDL_DestroyWindow(window);

    // Quit SDL subsystems
    SDL_Quit();

    return 0;
}
