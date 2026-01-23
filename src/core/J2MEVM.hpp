#pragma once

#include "VMConfig.hpp"
#include "Interpreter.hpp"
#include "RuntimeTypes.hpp"
#include <memory>
#include <string>

namespace j2me {
namespace core {

class J2MEVM {
public:
    J2MEVM();
    ~J2MEVM();

    int run(const VMConfig& config);

private:
    void setupLoaders(const VMConfig& config);
    bool loadMainClass(const VMConfig& config);
    void runHeadless();
    void runMIDlet();
    
    // Helpers
    bool isMIDletClass(std::shared_ptr<JavaClass> cls);
    bool hasMainMethod(std::shared_ptr<JavaClass> cls);
    void findAndRunInit();
    void findAndRunStartApp();
    void vmLoop();

    std::unique_ptr<Interpreter> interpreter;
    std::shared_ptr<JavaClass> mainClass;
    JavaObject* midletInstance = nullptr;
    
    // Keep config for reference during run
    VMConfig currentConfig;
};

}
}
