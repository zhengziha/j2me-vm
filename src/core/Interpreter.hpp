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
    
    void setStubLoader(std::shared_ptr<j2me::loader::JarLoader> loader) { stubLoader = loader; }

private:
    j2me::loader::JarLoader& jarLoader;
    std::shared_ptr<j2me::loader::JarLoader> stubLoader;
    std::map<std::string, std::shared_ptr<JavaClass>> loadedClasses;

    bool executeInstruction(std::shared_ptr<StackFrame> frame, util::DataReader& codeReader, std::optional<JavaValue>& returnVal);
    
    // Execute static initializer for a class
    void initializeClass(std::shared_ptr<JavaClass> cls);
    
    // Validate that a string is actually a class name, not a descriptor
    bool isValidClassName(const std::string& name);
};

} // namespace core
} // namespace j2me
