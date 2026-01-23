#pragma once

#include "StackFrame.hpp"
#include "../util/DataReader.hpp"
#include "../loader/JarLoader.hpp"
#include "RuntimeTypes.hpp"
#include "NativeRegistry.hpp"
#include "JavaThread.hpp"
#include <memory>
#include <map>
#include <optional>


namespace j2me {
namespace natives {
    void registerMediaNatives(j2me::core::NativeRegistry& registry);
}

namespace core {

class Interpreter {
public:
    Interpreter(j2me::loader::JarLoader& loader);
    void execute(std::shared_ptr<JavaThread> thread, int instructions);
    std::shared_ptr<JavaClass> resolveClass(const std::string& className);
    
    // Directly register a class (useful for .class files loaded directly)
    void registerClass(const std::string& className, std::shared_ptr<JavaClass> cls);
    
    void setLibraryLoader(std::shared_ptr<j2me::loader::JarLoader> loader) { libraryLoader = loader; }

private:
    j2me::loader::JarLoader& jarLoader;
    std::shared_ptr<j2me::loader::JarLoader> libraryLoader;
    std::map<std::string, std::shared_ptr<JavaClass>> loadedClasses;

    bool executeInstruction(std::shared_ptr<JavaThread> thread, std::shared_ptr<StackFrame> frame, util::DataReader& codeReader);
    
    // Execute static initializer for a class
    // Returns true if initialization was triggered (and caller should rewind PC/retry)
    bool initializeClass(std::shared_ptr<JavaThread> thread, std::shared_ptr<JavaClass> cls);
    
    // Validate that a string is actually a class name, not a descriptor
    bool isValidClassName(const std::string& name);

    // Make friends for native access
    friend class j2me::core::NativeRegistry;
    friend void j2me::natives::registerMediaNatives(j2me::core::NativeRegistry& registry);

    using InstructionHandler = std::function<bool(std::shared_ptr<JavaThread>, std::shared_ptr<StackFrame>, util::DataReader&, uint8_t)>;
    std::vector<InstructionHandler> instructionTable;
    void initInstructionTable();

    // Instruction group initializers
    void initConstants();
    void initLoads();
    void initStores();
    void initStack();
    void initMath();
    void initConversions();
    void initComparisons();
    void initControl();
    void initReferences();
    void initExtended();
};

} // namespace core
} // namespace j2me
