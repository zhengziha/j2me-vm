#include "J2MEVM.hpp"
#include "ClassParser.hpp"
#include "EventLoop.hpp"
#include "HeapManager.hpp"
#include "Logger.hpp"
#include "NativeRegistry.hpp"
#include "ThreadManager.hpp"
#include "TimerManager.hpp"
#include "Diagnostics.hpp"
#include "../native/javax_microedition_lcdui_Display.hpp"
#include "../native/java_lang_String.hpp"
#include "../platform/GraphicsContext.hpp"
#include "../util/FileUtils.hpp"
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

namespace j2me {
namespace core {

static void abortOnUnhandledException(const std::string& where, const std::string& what) {
    LOG_ERROR("Unhandled exception in " + where + ": " + what);
    EventLoop::getInstance().requestExit("unhandled exception in " + where + ": " + what);
}

J2MEVM::J2MEVM() {}
J2MEVM::~J2MEVM() {}

int J2MEVM::run(const VMConfig& config) {
    currentConfig = config;
    
    // 设置日志级别
    // Set log level
    Logger::getInstance().setLevel(config.logLevel);

    LOG_INFO("Starting J2ME VM Thread...");

    // 设置 Loader 和 Interpreter
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

    try {
        bool isMIDlet = isMIDletClass(mainClass);
        bool hasMain = hasMainMethod(mainClass);
        bool isJAR = !config.isClass;

        if (isJAR) {
            // JAR包执行优先级：Main-Class.main > MIDlet-1
            if (hasMain) {
                LOG_INFO("JAR mode: Running Main-Class.main method");
                runHeadless();
            } else if (isMIDlet) {
                LOG_INFO("JAR mode: Running as MIDlet");
                runMIDlet();
            } else {
                LOG_ERROR("JAR has no Main-Class.main and is not a MIDlet. Cannot run.");
                return 1;
            }
        } else {
            // 单个class文件执行
            if (hasMain) {
                LOG_INFO("Class mode: Running main method (headless)");
                runHeadless();
            } else if (isMIDlet || isDisplayableClass(mainClass)) {
                LOG_INFO("Class mode: Running as J2ME application");
                runMIDlet();
            } else {
                LOG_ERROR("Class is not a MIDlet/Displayable and has no main method. Cannot run.");
                return 1;
            }
        }
    } catch (const std::exception& e) {
        abortOnUnhandledException("J2MEVM::run", e.what());
        NativeRegistry::getInstance().setInterpreter(nullptr);
        return 1;
    } catch (...) {
        abortOnUnhandledException("J2MEVM::run", "<non-std exception>");
        NativeRegistry::getInstance().setInterpreter(nullptr);
        return 1;
    }

    if (Diagnostics::getInstance().getUncaughtExceptionCount() > 0) {
        LOG_ERROR("VM exiting due to uncaught exception: " + Diagnostics::getInstance().getLastUncaughtException());
        NativeRegistry::getInstance().setInterpreter(nullptr);
        return 1;
    }

    LOG_INFO("VM Thread Exiting...");
    NativeRegistry::getInstance().setInterpreter(nullptr);
    return 0;
}

bool J2MEVM::loadMainClass(const VMConfig& config) {
    if (config.isClass) {
        // 加载单个 .class 文件
        // Load single .class file
        LOG_INFO("Loading .class file: " + config.filePath);
        if (!config.classData) {
            LOG_ERROR("Class data missing for .class file");
            return false;
        }

        ClassParser parser;
        auto classFile = parser.parse(*config.classData);
        LOG_INFO("Successfully parsed class file!");

        mainClass = std::make_shared<JavaClass>(classFile);

        // 链接类 (处理父类)
        // Link class (handle superclass)
        // 确保父类被正确解析和链接
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
        // JAR 模式
        // JAR Mode
        // Interpreter 负责通过 loader 加载，我们只需要解析主类
        // Interpreter handles loading via loader, we just need to resolve the main class
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
    LOG_INFO("Exiting VM Event Loop...");
}

void J2MEVM::runHeadless() {
    LOG_INFO("Running main method (headless mode)...");
    
    // 查找 public static void main(String[] args) 方法
    // Find public static void main(String[] args)
    for (const auto& method : mainClass->rawFile->methods) {
        auto name = std::dynamic_pointer_cast<ConstantUtf8>(mainClass->rawFile->constant_pool[method.name_index]);
        auto desc = std::dynamic_pointer_cast<ConstantUtf8>(mainClass->rawFile->constant_pool[method.descriptor_index]);
        
        if (name && desc && name->bytes == "main" && desc->bytes == "([Ljava/lang/String;)V") {
            auto frame = std::make_shared<StackFrame>(method, mainClass->rawFile);
            
            // 创建 args 数组并填充参数
            // Create args array and populate with arguments
            auto arrayCls = interpreter->resolveClass("[Ljava/lang/String;");
            if (arrayCls) {
                auto arrayObj = HeapManager::getInstance().allocate(arrayCls);
                size_t argCount = currentConfig.mainMethodArgs.size();
                arrayObj->fields.resize(argCount);
                
                // 将每个参数转换为 Java String 对象
                // Convert each argument to Java String object
                auto stringCls = interpreter->resolveClass("java/lang/String");
                for (size_t i = 0; i < argCount && stringCls; i++) {
                    auto stringObj = j2me::natives::createJavaString(interpreter.get(), currentConfig.mainMethodArgs[i]);
                    arrayObj->fields[i] = reinterpret_cast<int64_t>(stringObj);
                }
                
                JavaValue vArgs;
                vArgs.type = JavaValue::REFERENCE;
                vArgs.val.ref = arrayObj;
                frame->setLocal(0, vArgs);
            }
            
            // 创建并启动主线程
            // Create and start main thread
            auto mainThread = std::make_shared<JavaThread>(frame);
            ThreadManager::getInstance().addThread(mainThread);
            
            // 主循环：执行指令直到线程结束
            // Main loop: execute instructions until thread finishes
            try {
                while (!mainThread->isFinished()) {
                    auto t = ThreadManager::getInstance().nextThread();
                    if (t) interpreter->execute(t, 20000);
                    ThreadManager::getInstance().removeFinishedThreads();

                    if (EventLoop::getInstance().shouldExit()) break;
                }
            } catch (const std::exception& e) {
                abortOnUnhandledException("headless main()", e.what());
                return;
            } catch (...) {
                abortOnUnhandledException("headless main()", "<non-std exception>");
                return;
            }
            
            if (EventLoop::getInstance().shouldExit()) {
                return;
            }

            LOG_INFO("main method completed.");
            return;
        }
    }
}

void J2MEVM::runMIDlet() {
    LOG_INFO("Running MIDlet (GUI mode)...");
    
    // 分配 MIDlet 实例
    // Allocate instance
    LOG_INFO("Allocating MIDlet instance");
    midletInstance = HeapManager::getInstance().allocate(mainClass);
    LOG_INFO("MIDlet instance allocated: " + std::to_string(reinterpret_cast<uintptr_t>(midletInstance)));

    // 运行构造函数 <init>
    // Run <init>
    LOG_INFO("Calling findAndRunInit()");
    findAndRunInit();
    
    // 运行 startApp()
    // Run startApp
    LOG_INFO("Calling findAndRunStartApp()");
    findAndRunStartApp();

    if (currentConfig.autoKeyEnabled) {
        LOG_INFO("Enabling auto keys");
        EventLoop::getInstance().scheduleAutoKeys(currentConfig.autoKeyCodes, currentConfig.autoKeyDelayMs, currentConfig.autoKeyPressMs, currentConfig.autoKeyBetweenKeysMs);
    }
    
    // 进入 VM 主循环
    // Enter VM Loop
    LOG_INFO("Calling vmLoop()");
    vmLoop();
    LOG_INFO("vmLoop() returned");
}

void J2MEVM::findAndRunInit() {
    // 在主类中查找 <init> (构造函数不继承，必须看类本身)
    // Find <init> in the main class (constructors are not inherited, must check the class itself)
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
             
             // 运行直到 <init> 完成
             // Run until <init> completes
             try {
                 while (!initThread->isFinished()) {
                     EventLoop::getInstance().pollSDL();
                     EventLoop::getInstance().dispatchEvents(interpreter.get());
                     TimerManager::getInstance().tick(interpreter.get());
                     EventLoop::getInstance().processRepaints(interpreter.get());

                     auto t = ThreadManager::getInstance().nextThread();
                     if (t) interpreter->execute(t, 10000);
                     ThreadManager::getInstance().removeFinishedThreads();
                 }
             } catch (const std::exception& e) {
                 abortOnUnhandledException("MIDlet <init>()", e.what());
                 return;
             } catch (...) {
                 abortOnUnhandledException("MIDlet <init>()", "<non-std exception>");
                 return;
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
    
    // 在类继承层次结构中查找 startApp
    // Find startApp in class hierarchy
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
                    
                    // 运行 startApp 直到完成 (或直到它 yield/return)
                    // Run startApp to completion (or until it yields/returns)
                    // 注意：在真实的 J2ME 中，startApp 可能很快返回，也可能阻塞。
                    // 这里我们运行直到它结束，同时处理事件。
                    // Note: In real J2ME, startApp might return quickly, or block.
                    // Here we run it until finished, pumping events.
                    try {
                        while (!startThread->isFinished()) {
                            if (EventLoop::getInstance().shouldExit()) break;
                            EventLoop::getInstance().pollSDL();
                            EventLoop::getInstance().dispatchEvents(interpreter.get());
                            TimerManager::getInstance().tick(interpreter.get());
                            EventLoop::getInstance().processRepaints(interpreter.get());
                            auto t = ThreadManager::getInstance().nextThread();
                            if (t) interpreter->execute(t, 10000);
                            ThreadManager::getInstance().removeFinishedThreads();
                        }
                    } catch (const std::exception& e) {
                        abortOnUnhandledException("MIDlet startApp()", e.what());
                        return;
                    } catch (...) {
                        abortOnUnhandledException("MIDlet startApp()", "<non-std exception>");
                        return;
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

    using clock = std::chrono::steady_clock;
    auto lastFrameTime = clock::now();
    const auto FRAME_INTERVAL = std::chrono::milliseconds(33); // ~30 FPS
    auto lastWatchdogTime = clock::now();
    uint64_t lastDrawCount = 0;
    uint64_t lastCommitCount = 0;

    // 移除了指令限速，依靠按键事件频率限制游戏速度
    // Throttling State Removed as per user request
    // We rely on Key Event Frequency Limiting for game speed control
#ifdef __SWITCH__
    LOG_INFO("Entering Switch main loop");
    int loopCount = 0;
    while (!eventLoop.shouldExit() && appletMainLoop()) {
        loopCount++;
        // if (loopCount % 100 == 0) {
        //     LOG_INFO("Switch main loop iteration: " + std::to_string(loopCount));
        // }
#else
    while (!eventLoop.shouldExit()) {
#endif
        try {
            auto now = clock::now();

            // 限制输入轮询和事件分发的频率为 30 FPS
            // J2ME 规范：重绘由 repaint() 触发，不是轮询式
            // Limit input polling and event dispatch to 30 FPS
            // J2ME spec: Repaint is triggered by repaint(), not polling
            if (now - lastFrameTime >= FRAME_INTERVAL) {
                eventLoop.pollSDL();
                eventLoop.dispatchEvents(interpreter.get());
                lastFrameTime = now;
            }

            // J2ME 规范：处理重绘请求 (由 repaint() 触发)
            // J2ME spec: Process repaint requests (triggered by repaint())
            // 同时提供连续渲染支持，因为某些游戏依赖此行为
            // Also provide continuous rendering support as some games depend on it
            eventLoop.processRepaints(interpreter.get());

            // 连续渲染：如果没有显式 repaint 请求，定期调用 paint 以支持依赖此行为的游戏
            // Continuous rendering: Call paint periodically if no explicit repaint request, for games that depend on it
            static auto lastContinuousPaintTime = clock::now();
            if (now - lastContinuousPaintTime >= FRAME_INTERVAL) {
                eventLoop.continuousPaint(interpreter.get());
                lastContinuousPaintTime = now;
            }

            TimerManager::getInstance().tick(interpreter.get());

            if (j2me::core::Logger::getInstance().getLevel() == j2me::core::LogLevel::DEBUG) {
                if (now - lastWatchdogTime >= std::chrono::seconds(1)) {
                    auto stats = ThreadManager::getInstance().getStats();
                    auto* displayable = j2me::natives::getCurrentDisplayable();
                    std::string displayableName = displayable && displayable->cls ? displayable->cls->name : "<null>";

                    auto& gc = j2me::platform::GraphicsContext::getInstance();
                    int64_t nowMs = gc.getNowMs();
                    uint64_t drawCount = gc.getDrawCount();
                    uint64_t commitCount = gc.getCommitCount();
                    uint64_t updateCount = gc.getUpdateCount();
                    uint64_t drawImgCount = gc.getDrawImageCount();
                    uint64_t drawRgbCount = gc.getDrawRGBCount();
                    uint64_t fillRectCount = gc.getFillRectCount();
                    uint64_t drawStrCount = gc.getDrawStringCount();
                    uint32_t lastDrawImgSample = gc.getLastDrawImageSampleARGB();
                    uint32_t lastDrawStrSample = gc.getLastDrawStringSampleARGB();
                    uint32_t lastFillRectColor = gc.getLastFillRectColorARGB();
                    uint32_t lastDrawImgMaxA = gc.getLastDrawImageMaxA();
                    uint32_t lastDrawImgNonZeroA = gc.getLastDrawImageNonZeroA();
                    int lastOpType = gc.getLastOpType();
                    int64_t lastOpMs = gc.getLastOpMs();
                    uint64_t sDrawImg = gc.getScreenDrawImageCount();
                    uint64_t sDrawRGB = gc.getScreenDrawRGBCount();
                    uint64_t sFillRect = gc.getScreenFillRectCount();
                    uint64_t sDrawStr = gc.getScreenDrawStringCount();
                    uint64_t drawRegionCount = gc.getDrawRegionCount();
                    uint64_t drawRegionNonZero = gc.getDrawRegionNonZeroTransformCount();
                    int lastDrawRegionTransform = gc.getLastDrawRegionTransform();
                    uint32_t sLastDrawImg = gc.getScreenLastDrawImageARGB();
                    uint32_t sLastDrawImgMaxA = gc.getScreenLastDrawImageMaxA();
                    uint32_t sLastDrawImgNonZeroA = gc.getScreenLastDrawImageNonZeroA();
                    uint32_t sLastDrawImgNonBlack = gc.getScreenLastDrawImageNonBlack();
                    uint32_t sLastFillRect = gc.getScreenLastFillRectARGB();
                    int sLastOp = gc.getScreenLastOpType();
                    int64_t sLastOpMs = gc.getScreenLastOpMs();
                    uint64_t backHash = gc.getBackHash();
                    uint64_t frontHash = gc.getFrontHash();
                    uint32_t backFmt = gc.getBackPixelFormat();
                    uint32_t frontFmt = gc.getFrontPixelFormat();
                    uint32_t backSample = gc.getBackSampleARGB();
                    uint32_t frontSample = gc.getFrontSampleARGB();
                    auto backClip = gc.getBackClipRect();
                    auto frameBounds = gc.getLastFrameBounds();
                    int64_t lastDrawMs = gc.getLastDrawMs();
                    int64_t lastCommitMs = gc.getLastCommitMs();
                    int64_t lastUpdateMs = gc.getLastUpdateMs();

                    auto& diag = j2me::core::Diagnostics::getInstance();
                    uint64_t flushCount = diag.getGameCanvasFlushCount();
                    uint64_t paintCommitCount = diag.getPaintCommitCount();
                    int64_t lastFlushMs = diag.getLastGameCanvasFlushMs();
                    int64_t lastPaintCommitMs = diag.getLastPaintCommitMs();
                    uint64_t resMiss = diag.getResourceNotFoundCount();
                    uint64_t imgDecFail = diag.getImageDecodeFailedCount();
                    std::string lastResMiss = diag.getLastResourceNotFound();
                    std::string lastImgDecFail = diag.getLastImageDecodeFailed();

                    LOG_DEBUG("[Watchdog] displayable=" + displayableName +
                              " threads=" + std::to_string(stats.total) +
                              " runnable=" + std::to_string(stats.runnable) +
                              " waiting=" + std::to_string(stats.waiting) +
                              " timed=" + std::to_string(stats.timedWaiting) +
                              " nowMs=" + std::to_string(nowMs) +
                              " draw=" + std::to_string(drawCount) +
                              " commit=" + std::to_string(commitCount) +
                              " update=" + std::to_string(updateCount) +
                              " drawImg=" + std::to_string(drawImgCount) +
                              " drawRGB=" + std::to_string(drawRgbCount) +
                              " fillRect=" + std::to_string(fillRectCount) +
                              " drawStr=" + std::to_string(drawStrCount) +
                              " lastDrawImgARGB=" + std::to_string(lastDrawImgSample) +
                              " lastDrawImgMaxA=" + std::to_string(lastDrawImgMaxA) +
                              " lastDrawImgNonZeroA=" + std::to_string(lastDrawImgNonZeroA) +
                              " lastDrawStrARGB=" + std::to_string(lastDrawStrSample) +
                              " lastFillRectARGB=" + std::to_string(lastFillRectColor) +
                              " lastOp=" + std::to_string(lastOpType) +
                              " ageLastOpMs=" + std::to_string(nowMs - lastOpMs) +
                              " sDrawImg=" + std::to_string(sDrawImg) +
                              " sDrawRGB=" + std::to_string(sDrawRGB) +
                              " sFillRect=" + std::to_string(sFillRect) +
                              " sDrawStr=" + std::to_string(sDrawStr) +
                              " drawRegion=" + std::to_string(drawRegionCount) +
                              " drawRegionNZ=" + std::to_string(drawRegionNonZero) +
                              " lastDrawRegionT=" + std::to_string(lastDrawRegionTransform) +
                              " sLastDrawImgARGB=" + std::to_string(sLastDrawImg) +
                              " sLastDrawImgMaxA=" + std::to_string(sLastDrawImgMaxA) +
                              " sLastDrawImgNonZeroA=" + std::to_string(sLastDrawImgNonZeroA) +
                              " sLastDrawImgNonBlack=" + std::to_string(sLastDrawImgNonBlack) +
                              " sLastFillRectARGB=" + std::to_string(sLastFillRect) +
                              " sLastOp=" + std::to_string(sLastOp) +
                              " ageSLastOpMs=" + std::to_string(nowMs - sLastOpMs) +
                              " flush=" + std::to_string(flushCount) +
                              " paintCommit=" + std::to_string(paintCommitCount) +
                              " resMiss=" + std::to_string(resMiss) +
                              " imgDecFail=" + std::to_string(imgDecFail) +
                              " lastResMiss=" + lastResMiss +
                              " lastImgDecFail=" + lastImgDecFail +
                              " backFmt=" + std::to_string(backFmt) +
                              " frontFmt=" + std::to_string(frontFmt) +
                              " backHash=" + std::to_string(backHash) +
                              " frontHash=" + std::to_string(frontHash) +
                              " backSampleARGB=" + std::to_string(backSample) +
                              " frontSampleARGB=" + std::to_string(frontSample) +
                              " backClip=" + std::to_string(backClip[0]) + "," + std::to_string(backClip[1]) + "," + std::to_string(backClip[2]) + "," + std::to_string(backClip[3]) +
                              " frameBounds=" + std::to_string(frameBounds[0]) + "," + std::to_string(frameBounds[1]) + "," + std::to_string(frameBounds[2]) + "," + std::to_string(frameBounds[3]) +
                              " lastDrawMs=" + std::to_string(lastDrawMs) +
                              " lastCommitMs=" + std::to_string(lastCommitMs) +
                              " lastUpdateMs=" + std::to_string(lastUpdateMs) +
                              " lastFlushMs=" + std::to_string(lastFlushMs) +
                              " lastPaintCommitMs=" + std::to_string(lastPaintCommitMs) +
                              " ageDrawMs=" + std::to_string(nowMs - lastDrawMs) +
                              " ageCommitMs=" + std::to_string(nowMs - lastCommitMs) +
                              " ageUpdateMs=" + std::to_string(nowMs - lastUpdateMs) +
                              " ageFlushMs=" + std::to_string(nowMs - lastFlushMs) +
                              " agePaintCommitMs=" + std::to_string(nowMs - lastPaintCommitMs));

                    if (drawCount != lastDrawCount && commitCount == lastCommitCount) {
                        LOG_DEBUG("[Watchdog] draw increased but commit unchanged");
                    }

                    lastDrawCount = drawCount;
                    lastCommitCount = commitCount;
                    lastWatchdogTime = now;
                }
            }
            
            auto thread = ThreadManager::getInstance().nextThread();
            if (thread) {
                // 执行一批指令
                // Execute a batch of instructions
                // 每批 20000 条指令提供良好的粒度
                // 20000 instructions per batch provides good granularity
                interpreter->execute(thread, 5000);
            } else {
                static int noThreadCount = 0;
                static int64_t lastLogTime = 0;
                auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();
                if (now - lastLogTime >= 1000) {  // Log every 1 second
                    LOG_INFO("[vmLoop] No threads to run (total count: " + std::to_string(++noThreadCount) + ")");
                    lastLogTime = now;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            ThreadManager::getInstance().removeFinishedThreads();
            
        } catch (const std::exception& e) {
             abortOnUnhandledException("VM loop", e.what());
             return;
        } catch (...) {
            abortOnUnhandledException("VM loop", "<non-std exception>");
            return;
        }
    }
    LOG_INFO("Exiting VM Event Loop...");
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

bool J2MEVM::isDisplayableClass(std::shared_ptr<JavaClass> cls) {
    if (!cls) return false;
    auto current = cls;
    while (current) {
        if (current->name == "javax/microedition/lcdui/Displayable") {
            return true;
        }
        current = current->superClass;
    }
    return false;
}

bool J2MEVM::needsGUI() {
    if (!mainClass) return false;
    return isMIDletClass(mainClass) || isDisplayableClass(mainClass);
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
