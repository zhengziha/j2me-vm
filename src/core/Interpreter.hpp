#pragma once

#include "StackFrame.hpp"
#include "../util/DataReader.hpp"
#include "../loader/JarLoader.hpp"
#include "RuntimeTypes.hpp"
#include "NativeRegistry.hpp"
#include <memory>
#include <map>
#include <optional>


namespace j2me {
namespace core {

class Interpreter {
public:
    Interpreter(j2me::loader::JarLoader& loader);
    std::optional<JavaValue> execute(std::shared_ptr<StackFrame> frame);
    std::shared_ptr<JavaClass> resolveClass(const std::string& className);
    
    void setSystemLoader(std::shared_ptr<j2me::loader::JarLoader> loader) { systemLoader = loader; }

private:
    j2me::loader::JarLoader& jarLoader;
    std::shared_ptr<j2me::loader::JarLoader> systemLoader;
    std::map<std::string, std::shared_ptr<JavaClass>> loadedClasses;

    bool executeInstruction(std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, std::optional<JavaValue>& returnVal);
    // std::shared_ptr<JavaClass> resolveClass(const std::string& className); // Made public
};

} // namespace core
} // namespace j2me
