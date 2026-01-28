#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <algorithm>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <csignal>
#include <sstream>

// 告诉 SDL2 我们会处理自己的入口点，不要将 main 替换为 SDL_main
#define SDL_MAIN_HANDLED

// 声明 main 函数，以便在 WinMain 中调用
int main(int argc, char* argv[]);

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// 取消定义可能与 LogLevel 冲突的宏
#undef ERROR
#undef NONE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int argc = 1;
    char* argv[] = {"j2me-vm.exe", nullptr};
    return main(argc, argv);
}
#endif

#include <SDL2/SDL.h>

#ifdef __SWITCH__
#include <switch.h>
#endif

#include "core/VMConfig.hpp"
#include "core/J2MEVM.hpp"
#include "core/Logger.hpp"
#include "core/EventLoop.hpp"
#include "core/ClassParser.hpp"
#include "platform/GraphicsContext.hpp"
#include "util/FileUtils.hpp"

// 辅助函数：检测类是否需要GUI（继承MIDlet或Displayable）
bool needsGUI(j2me::loader::JarLoader* loader, const std::string& className) {
    try {
        std::string clsName = className;
        if (clsName.size() >= 6 && clsName.substr(clsName.size()-6) == ".class") 
            clsName = clsName.substr(0, clsName.size()-6);
        
        std::replace(clsName.begin(), clsName.end(), '.', '/');
        std::string classPath = clsName + ".class";
        
        auto classData = loader->getFile(classPath);
        if (!classData) return false;
        
        j2me::core::ClassParser parser;
        auto classFile = parser.parse(*classData);
        
        auto currentClass = classFile;
        while (true) {
            if (currentClass->this_class == 0) break;
            auto thisClass = std::dynamic_pointer_cast<j2me::core::ConstantClass>(
                currentClass->constant_pool[currentClass->this_class]);
            if (!thisClass) break;
            
            auto nameInfo = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(
                currentClass->constant_pool[thisClass->name_index]);
            if (!nameInfo) break;
            
            std::string name = nameInfo->bytes;
            if (name == "javax/microedition/midlet/MIDlet" || 
                name == "javax/microedition/lcdui/Displayable") {
                return true;
            }
            
            if (currentClass->super_class == 0) break;
            auto superClass = std::dynamic_pointer_cast<j2me::core::ConstantClass>(
                currentClass->constant_pool[currentClass->super_class]);
            if (!superClass) break;
            
            auto superNameInfo = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(
                currentClass->constant_pool[superClass->name_index]);
            if (!superNameInfo) break;
            
            std::string superName = superNameInfo->bytes;
            if (superName == "java/lang/Object") break;

            if (superName == "javax/microedition/midlet/MIDlet" || 
                superName == "javax/microedition/lcdui/Displayable") {
                return true;
            }
            std::string superClassPath = superName + ".class";
            auto superClassData = loader->getFile(superClassPath);
            if (!superClassData) break;
            
            currentClass = parser.parse(*superClassData);
        }
    } catch (...) {
        return false;
    }
    return false;
}

// 辅助函数：解析 Manifest 文件以获取 MIDlet-1 类名
// Helper to parse Manifest for MIDlet-1
std::string parseMidletClassName(const std::string& manifestContent) {
    std::string mainClassLine;
    std::string midlet1Line;
    size_t pos = 0;
    while (pos < manifestContent.length()) {
        size_t end = manifestContent.find('\n', pos);
        if (end == std::string::npos) end = manifestContent.length();
        std::string line = manifestContent.substr(pos, end - pos);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.rfind("Main-Class:", 0) == 0) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                mainClassLine = line.substr(colonPos + 1);
                std::cerr << "DEBUG: Found Main-Class line, value: '" << mainClassLine << "'" << std::endl;
                break;
            }
        }
        if (line.rfind("MIDlet-1:", 0) == 0) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                midlet1Line = line.substr(colonPos + 1);
            }
        }
        pos = end + 1;
    }
    
    if (!mainClassLine.empty()) {
        std::string className = mainClassLine;
        size_t start = className.find_first_not_of(" \t");
        if (start != std::string::npos) className = className.substr(start);
        size_t end = className.find_last_not_of(" \t\r\n");
        if (end != std::string::npos) className = className.substr(0, end + 1);
        std::replace(className.begin(), className.end(), '.', '/');
        std::cerr << "DEBUG: Returning main class: '" << className << ".class'" << std::endl;
        return className + ".class";
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
    std::cerr << "DEBUG: No main class found, returning empty string" << std::endl;
    return "";
}

static std::vector<std::string> splitCommaTokens(const std::string& s) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, ',')) {
        size_t start = token.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = token.find_last_not_of(" \t\r\n");
        out.push_back(token.substr(start, end - start + 1));
    }
    return out;
}

static int parseAutoKeyToken(const std::string& t) {
    std::string s = t;
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    if (s == "fire" || s == "enter" || s == "ok") return -5;
    if (s == "soft1" || s == "softleft" || s == "leftsoft" || s == "lsoft") return -6;
    if (s == "soft2" || s == "softright" || s == "rightsoft" || s == "rsoft") return -7;
    if (s == "up") return -1;
    if (s == "down") return -2;
    if (s == "left") return -3;
    if (s == "right") return -4;
    if (s.size() == 1 && s[0] >= '0' && s[0] <= '9') return (int)s[0];
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: j2me-vm [--debug] [--log-level LEVEL] [--timeout-ms MS] [--auto-key [SEQ]] [--no-auto-key] [--auto-key-delay-ms MS] <path_to_jar_or_class>" << std::endl;
        std::cout << "  --debug: Enable debug mode (equivalent to --log-level debug)" << std::endl;
        std::cout << "  LEVEL: debug, info, error, none (default: info)" << std::endl;
        std::cout << "  MS: auto exit after MS milliseconds (0 disables, minimum: 15000)" << std::endl;
        std::cout << "  SEQ: comma-separated keys, e.g. soft1,fire or fire (default: soft1,fire when enabled)" << std::endl;
        return 1;
    }
    // 解析命令行参数
    // Parse command line arguments
    j2me::core::VMConfig config;
    config.logLevel = j2me::core::LogLevel::INFO;

#ifdef __SWITCH__
    LOG_INFO("Initializing romfs");
    romfsInit();
    LOG_INFO("Changing directory to romfs:/");
    chdir("romfs:/");
    LOG_INFO("romfs initialized successfully");
    config.filePath = "fr.jar";
#endif

    int64_t timeoutMs = 0;
    constexpr int64_t MIN_TIMEOUT_MS = 15000;
    bool autoKeyForcedOn = false;
    bool autoKeyForcedOff = false;
    std::string autoKeySeq;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        std::cerr << "DEBUG: Processing arg[" << i << "] = '" << arg << "', starts with '-': " << (arg[0] == '-' ? "true" : "false") << std::endl;
        std::cerr << "DEBUG: config.filePath = '" << config.filePath << "', config.mainMethodArgs.size() = " << config.mainMethodArgs.size() << std::endl;
        if (arg == "--debug") {
            config.logLevel = j2me::core::LogLevel::DEBUG;
        } else if (arg == "--log-level" && i + 1 < argc) {
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
        } else if (arg == "--timeout-ms" && i + 1 < argc) {
            timeoutMs = std::stoll(argv[++i]);
            if (timeoutMs < 0) timeoutMs = 0;
            if (timeoutMs > 0 && timeoutMs < MIN_TIMEOUT_MS) timeoutMs = MIN_TIMEOUT_MS;
        } else if (arg == "--auto-key") {
            autoKeyForcedOn = true;
            if (i + 1 < argc) {
                std::string next = argv[i + 1];
                if (!next.empty() && next[0] != '-') {
                    autoKeySeq = next;
                    i++;
                }
            }
        } else if (arg == "--no-auto-key") {
            autoKeyForcedOff = true;
        } else if (arg == "--auto-key-delay-ms" && i + 1 < argc) {
            config.autoKeyDelayMs = std::stoll(argv[++i]);
            if (config.autoKeyDelayMs < 0) config.autoKeyDelayMs = 0;
        } else if (arg[0] != '-') {
            if (config.filePath.empty()) {
                config.filePath = arg;
            } else {
                config.mainMethodArgs.push_back(arg);
            }
        }
    }

    if (config.filePath.empty()) {
        std::cerr << "Error: No file specified" << std::endl;
        return 1;
    }

    if (!autoKeyForcedOff && autoKeyForcedOn) {
        config.autoKeyEnabled = true;
        std::string seq = autoKeySeq.empty() ? "soft1,fire" : autoKeySeq;
        for (const auto& token : splitCommaTokens(seq)) {
            int key = parseAutoKeyToken(token);
            if (key != 0) config.autoKeyCodes.push_back(key);
        }
        if (config.autoKeyCodes.empty()) {
            config.autoKeyCodes = {-6, -5};
        }
    }

    // 设置全局日志级别
    // Set log level global
    j2me::core::Logger::getInstance().setLevel(config.logLevel);

    std::signal(SIGINT, [](int) {
        j2me::core::EventLoop::getInstance().requestExit("manual: SIGINT");
    });
    std::signal(SIGTERM, [](int) {
        j2me::core::EventLoop::getInstance().requestExit("manual: SIGTERM");
    });
    
    // 检查是否为 .class 文件
    // Check if it's a .class file
    std::cerr << "DEBUG: Checking if '" << config.filePath << "' is a .class file" << std::endl;
    config.isClass = j2me::util::FileUtils::isClassFile(config.filePath);
    std::cerr << "DEBUG: isClassFile result: " << (config.isClass ? "true" : "false") << std::endl;
    
    // 加载 Loader
    // Load Loaders
    // appLoader 用于加载应用程序的类 (JAR 或 class)
    // libraryLoader 用于加载系统类库 (rt.jar)
    config.appLoader = std::make_shared<j2me::loader::JarLoader>();
    config.libraryLoader = std::make_shared<j2me::loader::JarLoader>();
    
    // 加载类库 (rt.jar)
    // Load Library
    bool libraryLoaded = false;
    std::vector<std::string> libraryPaths;
    
    if (!config.filePath.empty()) {
        size_t lastSlash = config.filePath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            std::string jarDir = config.filePath.substr(0, lastSlash);
            libraryPaths.push_back(jarDir + "/rt.jar");
            libraryPaths.push_back(jarDir + "/stubs/rt.jar");
            libraryPaths.push_back(jarDir + "/../stubs/rt.jar");
        }
    }
    // 添加Switch平台特有的路径
    #ifdef __SWITCH__
    libraryPaths.push_back("romfs/rt.jar");
    libraryPaths.push_back("rt.jar");
    #endif
    // 添加通用路径
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
    
    std::cerr << "DEBUG: isClass = " << (config.isClass ? "true" : "false") << std::endl;
    
    if (config.isClass) {
        std::cerr << "DEBUG: Entering .class mode" << std::endl;
        config.classData = j2me::util::FileUtils::readFile(config.filePath);
        if (!config.classData) {
            LOG_ERROR("Failed to read .class file: " + config.filePath);
            return 1;
        }
        
        // 解析类名
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
        std::cerr << "DEBUG: Set mainClassName to: " << config.mainClassName << std::endl;
        
    } else {
        std::cerr << "DEBUG: Entering JAR mode" << std::endl;
        // JAR 模式
        // JAR Mode
        LOG_INFO("Loading JAR: " + config.filePath);
        if (!config.appLoader->load(config.filePath)) {
            LOG_ERROR("Failed to load JAR file.");
            return 1;
        }
        
        auto manifest = config.appLoader->getManifest();
        if (manifest) {
            LOG_INFO("Manifest found:\n" + *manifest);
            std::cerr << "DEBUG: About to call parseMidletClassName" << std::endl;
            std::string midletClass = parseMidletClassName(*manifest);
            std::cerr << "DEBUG: parseMidletClassName returned: '" << midletClass << "'" << std::endl;
            if (!midletClass.empty()) {
                config.mainClassName = midletClass;
                LOG_INFO("Found main class from manifest: " + config.mainClassName);
            } else {
                LOG_INFO("No Main-Class or MIDlet-1 found in manifest");
            }
        } else {
             LOG_INFO("No Manifest found.");
        }
    }

    if (timeoutMs > 0) {
        std::thread([timeoutMs]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
            j2me::core::EventLoop::getInstance().requestExit("timeout: " + std::to_string(timeoutMs) + "ms");
        }).detach();
        LOG_INFO("[Exit] Auto-timeout enabled: " + std::to_string(timeoutMs) + "ms");
    }

    // 检查是否需要GUI
    bool needsWindow = false;
    if (!config.mainClassName.empty()) {
        needsWindow = needsGUI(config.appLoader.get(), config.mainClassName);
        LOG_INFO("GUI required: " + std::string(needsWindow ? "true" : "false"));
    }

    // 初始化SDL和窗口（仅当需要时）
    SDL_Window* window = nullptr;
    if (needsWindow) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            LOG_ERROR("SDL initialization failed: " + std::string(SDL_GetError()));
            return 1;
        }
        
        #ifdef __SWITCH__
        romfsInit();
        LOG_INFO("Initialized romfs");
        #endif

        window = SDL_CreateWindow("J2ME VM", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 240, 320, SDL_WINDOW_SHOWN);
        if (!window) {
            LOG_ERROR("Window creation failed: " + std::string(SDL_GetError()));
            SDL_Quit();
            return 1;
        }

        j2me::platform::GraphicsContext::getInstance().init(window, 240, 320);
        
        LOG_INFO("SDL and window initialized successfully");
    }

    // 运行虚拟机
    // Run VM
    LOG_INFO("Starting VM with config: filePath='" + config.filePath + "', mainClassName='" + config.mainClassName + "'");
    int result = 1;

    try {
        j2me::core::J2MEVM vm;
        LOG_INFO("Calling vm.run()");
        result = vm.run(config);
        LOG_INFO("VM run completed with result: " + std::to_string(result));
        std::cout << "[INFO] VM run completed with result: " << result << std::endl;
    } catch (const std::exception& e) {
        LOG_ERROR("VM crashed with exception: " + std::string(e.what()));
        std::cerr << "[ERROR] VM crashed with exception: " << e.what() << std::endl;
        // 尝试将错误信息写入文件
        std::ofstream errorFile("j2me-vm-error.log");
        if (errorFile.is_open()) {
            errorFile << "VM crashed with exception: " << e.what() << std::endl;
            errorFile.close();
        }
    } catch (...) {
        LOG_ERROR("VM crashed with unknown exception");
        std::cerr << "[ERROR] VM crashed with unknown exception" << std::endl;
        // 尝试将错误信息写入文件
        std::ofstream errorFile("j2me-vm-error.log");
        if (errorFile.is_open()) {
            errorFile << "VM crashed with unknown exception" << std::endl;
            errorFile.close();
        }
    }
    
    // 清理SDL（仅当初始化过时）
    if (needsWindow) {
        LOG_INFO("VM Stopped. Cleaning up SDL.");
        if (window) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
        #ifdef __SWITCH__
        romfsExit();
        LOG_INFO("Exited romfs");
        #endif
    } else {
        LOG_INFO("VM Stopped (headless mode, no SDL cleanup needed).");
    }
    
    #ifdef __SWITCH__
    romfsExit();
    LOG_INFO("Exited romfs");
    #endif

    LOG_INFO("Shutdown complete. Exiting with code: " + std::to_string(result));
    return result;
}
