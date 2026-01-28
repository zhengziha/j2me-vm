#pragma once

#include "VMConfig.hpp"
#include "Interpreter.hpp"
#include "RuntimeTypes.hpp"
#include <memory>
#include <string>

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace j2me {
namespace core {

class J2MEVM {
public:
    J2MEVM();
    ~J2MEVM();

    // Run the VM with the given configuration
    // 使用给定配置运行虚拟机
    int run(const VMConfig& config);

private:
    // Setup class loaders (app loader and library loader)
    // 设置类加载器 (应用加载器和系统库加载器)
    void setupLoaders(const VMConfig& config);

    // Load the main class specified in config
    // 加载配置中指定的主类
    bool loadMainClass(const VMConfig& config);

    // Run in headless mode (standard public static void main)
    // 以无头模式运行 (标准的 public static void main)
    void runHeadless();

    // 运行作为 MIDlet 运行 (J2ME 生命周期: init, startApp 等)
    // 作为 MIDlet 运行 (J2ME 生命周期: init, startApp 等)
    void runMIDlet();
    
    // 判断是否需要图形界面 (MIDlet 或 Displayable)
    // 判断是否需要图形界面 (MIDlet 或 Displayable)
    bool needsGUI();
    
    // Helpers
    // 辅助函数: 判断是否为 MIDlet 类
    bool isMIDletClass(std::shared_ptr<JavaClass> cls);
    // 辅助函数: 判断是否为 Displayable 类
    bool isDisplayableClass(std::shared_ptr<JavaClass> cls);
    // 辅助函数: 判断是否有 main 方法
    bool hasMainMethod(std::shared_ptr<JavaClass> cls);
    // 查找并运行构造函数 <init>
    void findAndRunInit();
    // 查找并运行 startApp() 方法
    void findAndRunStartApp();
    // 虚拟机主事件循环
    void vmLoop();

    // Interpreter instance
    // 解释器实例
    std::unique_ptr<Interpreter> interpreter;

    // Main class reference
    // 主类引用
    std::shared_ptr<JavaClass> mainClass;

    // MIDlet instance (if running as MIDlet)
    // MIDlet 实例 (如果作为 MIDlet 运行)
    JavaObject* midletInstance = nullptr;
    
    // Keep config for reference during run
    // 保存配置以供运行时参考
    VMConfig currentConfig;
};

}
}
