#include <iostream>
#include <SDL2/SDL.h>
#include <algorithm> // for std::replace
#include "loader/JarLoader.hpp"
#include "core/ClassParser.hpp"
#include "core/Interpreter.hpp"
#include "core/HeapManager.hpp"
#include "platform/GraphicsContext.hpp"
// #include "native/javax_microedition_lcdui_Graphics.cpp" // Don't include .cpp!
#include "native/javax_microedition_lcdui_Graphics.hpp" 
#include "native/javax_microedition_lcdui_Display.hpp"
#include "native/java_lang_Class.hpp"
#include "native/java_lang_Object.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: j2me-vm <path_to_jar>" << std::endl;
        return 1;
    }

    // Register natives
    j2me::natives::registerGraphicsNatives();
    j2me::natives::registerDisplayNatives();
    j2me::natives::registerClassNatives();
    j2me::natives::registerObjectNatives();

    std::string jarPath = argv[1];
    std::cout << "Starting J2ME VM..." << std::endl;
    std::cout << "Loading JAR: " << jarPath << std::endl;

    j2me::loader::JarLoader loader;
    if (!loader.load(jarPath)) {
        std::cerr << "Failed to load JAR file." << std::endl;
        return 1;
    }
    j2me::core::NativeRegistry::getInstance().setJarLoader(&loader);

    auto manifest = loader.getManifest();
    if (manifest) {
        std::cout << "Manifest found:\n" << *manifest << std::endl;
    } else {
        std::cout << "No Manifest found." << std::endl;
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        // Just print error and continue if headless, but we really need it for InputTest
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        // return 1; // Don't return, maybe we can run in headless mode? 
        // But InputTest needs events.
        // In the sandbox, maybe we have a dummy driver?
        // Let's assume we can proceed or mock it if fails.
    } else {
        std::cout << "SDL Initialized." << std::endl;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("J2ME Emulator",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          240, 320, // Standard J2ME resolution
                                          SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        // Continue anyway for now...
    } else {
        std::cout << "Window Created." << std::endl;
    }
    
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
                     std::cout << "Found MIDlet-1 class: " << mainClassName << std::endl;
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
            mainClassName = "ImageTest.class";
        }
    }

    // Load system classes (rt.jar)
    auto systemLoader = std::make_shared<j2me::loader::JarLoader>();
    if (systemLoader->load("rt.jar")) {
        std::cout << "Loaded system classes (rt.jar)" << std::endl;
    } else {
        std::cerr << "Warning: rt.jar not found. System classes might be missing." << std::endl;
    }

    std::shared_ptr<j2me::core::JavaObject> midletInstance;
    j2me::core::Interpreter interpreter(loader);
    interpreter.setSystemLoader(systemLoader);

    if (loader.hasFile(mainClassName)) {
        auto classData = loader.getFile(mainClassName);
        if (classData) {
            std::cout << "Found " << mainClassName << ", parsing..." << std::endl;
            try {
                j2me::core::ClassParser parser;
                auto classFile = parser.parse(*classData);
                std::cout << "Successfully parsed class file!" << std::endl;
                
                // For GraphicsTest, we want to create instance and call paint(Graphics)
                // 1. Create instance (ALLOC)
                std::cout << "Resolving main class..." << std::endl;
                auto cls = interpreter.resolveClass(mainClassName.substr(0, mainClassName.length()-6)); // remove .class
                std::cout << "Allocating instance..." << std::endl;
                midletInstance.reset(j2me::core::HeapManager::getInstance().allocate(cls));

                // 2. Call constructor (init) if needed
                // We assume 0-arg constructor for GraphicsTest
                // We need to find <init> method
                std::cout << "Finding <init>..." << std::endl;
                for (const auto& method : classFile->methods) {
                    auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(classFile->constant_pool[method.name_index]);
                    if (name->bytes == "<init>") {
                         std::cout << "Executing <init>..." << std::endl;
                         auto frame = std::make_shared<j2me::core::StackFrame>(method, classFile);
                         // Set 'this'
                         j2me::core::JavaValue vThis; vThis.type = j2me::core::JavaValue::REFERENCE; vThis.val.ref = midletInstance.get();
                         frame->setLocal(0, vThis);
                         interpreter.execute(frame);
                         break;
                    }
                }
                std::cout << "<init> done." << std::endl;
                
                // 3. Call startApp() if it is a MIDlet
                // Check inheritance (simplified)
                // Assuming it is a MIDlet for now if it has startApp
                for (const auto& method : classFile->methods) {
                    auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(classFile->constant_pool[method.name_index]);
                    if (name->bytes == "startApp") {
                         std::cout << "Calling startApp()..." << std::endl;
                         auto frame = std::make_shared<j2me::core::StackFrame>(method, classFile);
                         j2me::core::JavaValue vThis; vThis.type = j2me::core::JavaValue::REFERENCE; vThis.val.ref = midletInstance.get();
                         frame->setLocal(0, vThis);
                         interpreter.execute(frame);
                         break;
                    }
                }
                
                // Let's run 'main' if it exists (for console tests)
                // Or if it is a Canvas, we'll drive it in the loop.
                
                bool hasMain = false;
                for (const auto& method : classFile->methods) {
                    auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(classFile->constant_pool[method.name_index]);
                    if (name->bytes == "main") {
                         auto frame = std::make_shared<j2me::core::StackFrame>(method, classFile);
                         interpreter.execute(frame);
                         hasMain = true;
                         break;
                    }
                }
                
                if (!hasMain) {
                     std::cout << "No main method, assuming Canvas/MIDlet..." << std::endl;
                }

            } catch (const std::exception& e) {
                std::cerr << "Failed to run class file: " << e.what() << std::endl;
            }
        }
    } else {
        std::cout << mainClassName << " not found in JAR" << std::endl;
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
                         std::cerr << "Warning: paint method not found in " << cls->name << std::endl;
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
    }

    // Destroy window
    SDL_DestroyWindow(window);

    // Quit SDL subsystems
    SDL_Quit();

    return 0;
}
