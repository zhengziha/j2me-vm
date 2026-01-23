#include "J2MEVM.hpp"
#include "ClassParser.hpp"
#include "EventLoop.hpp"
#include "HeapManager.hpp"
#include "Logger.hpp"
#include "NativeRegistry.hpp"
#include "ThreadManager.hpp"
#include "TimerManager.hpp"
#include "../util/FileUtils.hpp"
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

namespace j2me {
namespace core {

J2MEVM::J2MEVM() {}
J2MEVM::~J2MEVM() {}

int J2MEVM::run(const VMConfig& config) {
    currentConfig = config;
    
    // Set log level
    Logger::getInstance().setLevel(config.logLevel);

    LOG_INFO("Starting J2ME VM Thread...");

    // Setup Loaders & Interpreter
    j2me::loader::JarLoader& baseLoader = config.isClass ? *config.libraryLoader : *config.appLoader;
    interpreter = std::make_unique<Interpreter>(baseLoader);
    interpreter->setLibraryLoader(config.libraryLoader);
    
    NativeRegistry::getInstance().setInterpreter(interpreter.get());
    if (!config.isClass && config.appLoader) {
        NativeRegistry::getInstance().setJarLoader(config.appLoader.get());
    }

    if (!loadMainClass(config)) {
        return 1;
    }

    bool isMIDlet = isMIDletClass(mainClass);
    bool hasMain = hasMainMethod(mainClass);

    if (hasMain && !isMIDlet) {
        runHeadless();
    } else if (isMIDlet) {
        runMIDlet();
    } else {
        LOG_ERROR("Class is not a MIDlet and has no main method. Cannot run.");
        return 1;
    }

    LOG_INFO("VM Thread Exiting...");
    NativeRegistry::getInstance().setInterpreter(nullptr);
    return 0;
}

bool J2MEVM::loadMainClass(const VMConfig& config) {
    if (config.isClass) {
        LOG_INFO("Loading .class file: " + config.filePath);
        if (!config.classData) {
            LOG_ERROR("Class data missing for .class file");
            return false;
        }

        ClassParser parser;
        auto classFile = parser.parse(*config.classData);
        LOG_INFO("Successfully parsed class file!");

        mainClass = std::make_shared<JavaClass>(classFile);

        // Link
        if (classFile->super_class != 0) {
            auto superInfo = std::dynamic_pointer_cast<ConstantClass>(classFile->constant_pool[classFile->super_class]);
            auto superNameInfo = std::dynamic_pointer_cast<ConstantUtf8>(classFile->constant_pool[superInfo->name_index]);
            std::string superName = superNameInfo->bytes;
            
            if (superName != "java/lang/Object") {
                auto superClass = interpreter->resolveClass(superName);
                mainClass->link(superClass);
            } else {
                mainClass->link(nullptr);
            }
        } else {
            mainClass->link(nullptr);
        }

        interpreter->registerClass(config.mainClassName, mainClass);
        LOG_INFO("Registered class: " + config.mainClassName);
        return true;
    } else {
        // JAR Mode
        // Interpreter handles loading via loader, we just need to resolve it
        try {
            std::string clsName = config.mainClassName;
            if (clsName.size() >= 6 && clsName.substr(clsName.size()-6) == ".class") 
                clsName = clsName.substr(0, clsName.size()-6);
            
            LOG_INFO("Resolving main class: " + clsName);
            mainClass = interpreter->resolveClass(clsName);
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to load main class: " + std::string(e.what()));
            return false;
        }
    }
}

void J2MEVM::runHeadless() {
    LOG_INFO("Running main method (headless mode)...");
    
    for (const auto& method : mainClass->rawFile->methods) {
        auto name = std::dynamic_pointer_cast<ConstantUtf8>(mainClass->rawFile->constant_pool[method.name_index]);
        auto desc = std::dynamic_pointer_cast<ConstantUtf8>(mainClass->rawFile->constant_pool[method.descriptor_index]);
        
        if (name && desc && name->bytes == "main" && desc->bytes == "([Ljava/lang/String;)V") {
            auto frame = std::make_shared<StackFrame>(method, mainClass->rawFile);
            
            auto arrayCls = interpreter->resolveClass("[Ljava/lang/String;");
            if (arrayCls) {
                auto arrayObj = HeapManager::getInstance().allocate(arrayCls);
                arrayObj->fields.resize(0);
                
                JavaValue vArgs;
                vArgs.type = JavaValue::REFERENCE;
                vArgs.val.ref = arrayObj;
                frame->setLocal(0, vArgs);
            }
            
            auto mainThread = std::make_shared<JavaThread>(frame);
            ThreadManager::getInstance().addThread(mainThread);
            
            while (!mainThread->isFinished()) {
                 auto t = ThreadManager::getInstance().nextThread();
                 if (t) interpreter->execute(t, 20000);
                 ThreadManager::getInstance().removeFinishedThreads();
                 
                 if (EventLoop::getInstance().shouldExit()) break;
            }
            
            LOG_INFO("main method completed.");
            return;
        }
    }
}

void J2MEVM::runMIDlet() {
    LOG_INFO("Running MIDlet (GUI mode)...");
    
    // Allocate instance
    midletInstance = HeapManager::getInstance().allocate(mainClass);

    // Run <init>
    findAndRunInit();
    
    // Run startApp
    findAndRunStartApp();
    
    // Enter VM Loop
    vmLoop();
}

void J2MEVM::findAndRunInit() {
    // Find <init> in the main class (constructors are not inherited, but we should look at the class itself)
    // Actually we should scan methods of mainClass
    bool found = false;
    for (const auto& method : mainClass->rawFile->methods) {
        auto name = std::dynamic_pointer_cast<ConstantUtf8>(mainClass->rawFile->constant_pool[method.name_index]);
        if (name->bytes == "<init>") {
             LOG_INFO("Executing <init>...");
             auto frame = std::make_shared<StackFrame>(method, mainClass->rawFile);
             JavaValue vThis; vThis.type = JavaValue::REFERENCE; vThis.val.ref = midletInstance;
             frame->setLocal(0, vThis);
             
             auto initThread = std::make_shared<JavaThread>(frame);
             ThreadManager::getInstance().addThread(initThread);
             
             while (!initThread->isFinished()) {
                  EventLoop::getInstance().pollSDL();
                  EventLoop::getInstance().dispatchEvents(interpreter.get());
                  TimerManager::getInstance().tick(interpreter.get());
                  EventLoop::getInstance().render(interpreter.get());
                  
                  auto t = ThreadManager::getInstance().nextThread();
                  if (t) interpreter->execute(t, 10000);
                  ThreadManager::getInstance().removeFinishedThreads();
             }
             found = true;
             break;
        }
    }
    if (!found) LOG_INFO("<init> not found or not executed.");
}

void J2MEVM::findAndRunStartApp() {
    auto currentCls = mainClass;
    bool startAppFound = false;
    
    while (currentCls) {
        for (const auto& method : currentCls->rawFile->methods) {
            auto name = std::dynamic_pointer_cast<ConstantUtf8>(currentCls->rawFile->constant_pool[method.name_index]);
            if (name->bytes == "startApp") {
                    LOG_INFO("Calling startApp() in class " + currentCls->name);
                    auto frame = std::make_shared<StackFrame>(method, currentCls->rawFile);
                    JavaValue vThis; vThis.type = JavaValue::REFERENCE; vThis.val.ref = midletInstance;
                    frame->setLocal(0, vThis);
                    
                    auto startThread = std::make_shared<JavaThread>(frame);
                    ThreadManager::getInstance().addThread(startThread);
                    
                    // Run startApp to completion (or until it yields/returns)
                    // Note: In real J2ME, startApp might return quickly, or block. 
                    // Here we run it until finished, pumping events.
                    while (!startThread->isFinished()) {
                             if (EventLoop::getInstance().shouldExit()) break;
                             EventLoop::getInstance().pollSDL();
                             EventLoop::getInstance().dispatchEvents(interpreter.get());
                             TimerManager::getInstance().tick(interpreter.get());
                             EventLoop::getInstance().render(interpreter.get());
                             auto t = ThreadManager::getInstance().nextThread();
                             if (t) interpreter->execute(t, 10000);
                             ThreadManager::getInstance().removeFinishedThreads();
                    }
                    
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
}

void J2MEVM::vmLoop() {
    LOG_INFO("Entering VM Event Loop...");
    EventLoop& eventLoop = EventLoop::getInstance();
    while (!eventLoop.shouldExit()) {
        try {
            eventLoop.pollSDL();
            eventLoop.dispatchEvents(interpreter.get());
            TimerManager::getInstance().tick(interpreter.get());
            eventLoop.render(interpreter.get());
            
            auto thread = ThreadManager::getInstance().nextThread();
            if (thread) {
                interpreter->execute(thread, 50000); 
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            ThreadManager::getInstance().removeFinishedThreads();
            
        } catch (const std::exception& e) {
             LOG_ERROR("Exception in VM Loop: " + std::string(e.what()));
             // std::exit(1); // Don't exit hard, maybe return?
             return;
        }
    }
}

bool J2MEVM::isMIDletClass(std::shared_ptr<JavaClass> cls) {
    if (!cls) return false;
    auto current = cls;
    while (current) {
        if (current->name == "javax/microedition/midlet/MIDlet") {
            return true;
        }
        current = current->superClass;
    }
    return false;
}

bool J2MEVM::hasMainMethod(std::shared_ptr<JavaClass> cls) {
    if (!cls || !cls->rawFile) return false;
    for (const auto& method : cls->rawFile->methods) {
        auto name = std::dynamic_pointer_cast<ConstantUtf8>(cls->rawFile->constant_pool[method.name_index]);
        auto desc = std::dynamic_pointer_cast<ConstantUtf8>(cls->rawFile->constant_pool[method.descriptor_index]);
        if (name && desc && name->bytes == "main" && desc->bytes == "([Ljava/lang/String;)V") {
            return true;
        }
    }
    return false;
}

}
}
