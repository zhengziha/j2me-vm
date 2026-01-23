#pragma once

#include <string>
#include <map>
#include <functional>
#include <memory>
#include "StackFrame.hpp"
#include "../loader/JarLoader.hpp"

namespace j2me {
namespace core {

class Interpreter;
class JavaThread;

// Native function signature: void(Thread, Frame)
// The native function pops arguments from the frame's stack and pushes the result (if any).
using NativeFunction = std::function<void(std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame>)>;

class NativeRegistry {
public:
    static NativeRegistry& getInstance() {
        static NativeRegistry instance;
        return instance;
    }

    void registerNative(const std::string& className, const std::string& methodName, const std::string& descriptor, NativeFunction func);
    NativeFunction getNative(const std::string& className, const std::string& methodName, const std::string& descriptor);

    void setJarLoader(j2me::loader::JarLoader* loader) { this->loader = loader; }
    j2me::loader::JarLoader* getJarLoader() { return loader; }
    
    void setInterpreter(Interpreter* interp) { this->interpreter = interp; }
    Interpreter* getInterpreter() { return interpreter; }

private:
    NativeRegistry();
    std::map<std::string, NativeFunction> registry;
    j2me::loader::JarLoader* loader = nullptr;
    Interpreter* interpreter = nullptr;
    
    std::string makeKey(const std::string& className, const std::string& methodName, const std::string& descriptor);
};

} // namespace core
} // namespace j2me
