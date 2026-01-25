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

#include "core/VMConfig.hpp"
#include "core/J2MEVM.hpp"
#include "core/Logger.hpp"
#include "core/EventLoop.hpp"
#include "platform/GraphicsContext.hpp"
#include "util/FileUtils.hpp"

// 辅助函数：解析 Manifest 文件以获取 MIDlet-1 类名
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
        std::cout << "Usage: j2me-vm [--log-level LEVEL] [--timeout-ms MS] [--auto-key [SEQ]] [--no-auto-key] [--auto-key-delay-ms MS] <path_to_jar_or_class>" << std::endl;
        std::cout << "  LEVEL: debug, info, error, none (default: info)" << std::endl;
        std::cout << "  MS: auto exit after MS milliseconds (0 disables, minimum: 15000)" << std::endl;
        std::cout << "  SEQ: comma-separated keys, e.g. soft1,fire or fire (default: soft1,fire when enabled)" << std::endl;
        return 1;
    }

    // 解析命令行参数
    // Parse command line arguments
    j2me::core::VMConfig config;
    config.logLevel = j2me::core::LogLevel::INFO;
    int64_t timeoutMs = 0;
    constexpr int64_t MIN_TIMEOUT_MS = 15000;
    bool autoKeyForcedOn = false;
    bool autoKeyForcedOff = false;
    std::string autoKeySeq;

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
            config.filePath = arg;
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

    // 初始化 SDL (必须在主线程中进行)
    // Initialize SDL (MUST BE ON MAIN THREAD)
    // SDL_INIT_VIDEO: 初始化视频子系统
    // SDL_INIT_AUDIO: 初始化音频子系统
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        LOG_ERROR("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
    } else {
        LOG_INFO("SDL Initialized.");
    }

    std::signal(SIGINT, [](int) {
        j2me::core::EventLoop::getInstance().requestExit("manual: SIGINT");
    });
    std::signal(SIGTERM, [](int) {
        j2me::core::EventLoop::getInstance().requestExit("manual: SIGTERM");
    });

    // 创建窗口
    // Create window
    // 初始大小设置为 3 倍缩放 (240x320) 以适应现代屏幕
    // Initial size set to 3x scale (240x320) for better visibility on modern screens   
    // SDL_WINDOW_ALLOW_HIGHDPI: 支持高 DPI 显示器 (如 Retina 屏幕)
    SDL_Window* window = SDL_CreateWindow("J2ME Emulator",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          240, 320, 
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == nullptr) {
        LOG_ERROR("Window could not be created! SDL_Error: " + std::string(SDL_GetError()));
        return 1;
    }
    LOG_INFO("Window Created.");

    SDL_SetWindowResizable(window, SDL_FALSE);
    SDL_SetWindowMinimumSize(window, 240, 320);
    SDL_SetWindowMaximumSize(window, 240, 320);
    
    // 初始化图形上下文，设置逻辑分辨率 (240x320)
    // Init Graphics Context with Logical Resolution (240x320)
    // 所有的绘图操作都基于这个逻辑分辨率，最后会缩放到实际窗口大小
    // All drawing operations are based on this logical resolution, and scaled to window size at the end
    j2me::platform::GraphicsContext::getInstance().init(window, 240, 320);
    
    // 检查是否为 .class 文件
    // Check if it's a .class file
    config.isClass = j2me::util::FileUtils::isClassFile(config.filePath);
    
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
        
    } else {
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
            std::string midletClass = parseMidletClassName(*manifest);
            if (!midletClass.empty()) {
                config.mainClassName = midletClass;
                LOG_INFO("Found MIDlet-1 class: " + config.mainClassName);
            }
        } else {
             LOG_INFO("No Manifest found.");
        }
        
        if (config.mainClassName.empty()) {
            // 启发式回退策略
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

    if (timeoutMs > 0) {
        std::thread([timeoutMs]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
            j2me::core::EventLoop::getInstance().requestExit("timeout: " + std::to_string(timeoutMs) + "ms");
        }).detach();
        LOG_INFO("[Exit] Auto-timeout enabled: " + std::to_string(timeoutMs) + "ms");
    }

    // 运行虚拟机
    // Run VM
    j2me::core::J2MEVM vm;
    int result = vm.run(config);
    
    LOG_INFO("VM Stopped. Cleaning up SDL.");

    SDL_DestroyWindow(window);
    SDL_Quit();
    
    LOG_INFO("Shutdown complete.");
    return result;
}
