#pragma once

#include <string>
#include "../core/RuntimeTypes.hpp"
#include "../core/Interpreter.hpp"

namespace j2me {
namespace core {
    class NativeRegistry;
}
namespace natives {

void registerStringNatives(j2me::core::NativeRegistry& registry);
void registerStringBuilderNatives();

// Helper to create a Java String object from C++ string
j2me::core::JavaObject* createJavaString(j2me::core::Interpreter* interpreter, const std::string& str);

// Helper to get C++ string from Java String object
std::string getJavaString(j2me::core::JavaObject* strObj);

}
}
