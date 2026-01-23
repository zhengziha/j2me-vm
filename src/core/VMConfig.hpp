#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "Logger.hpp"
#include "../loader/JarLoader.hpp"

namespace j2me {
namespace core {

struct VMConfig {
    std::string filePath;
    LogLevel logLevel = LogLevel::INFO;
    std::shared_ptr<j2me::loader::JarLoader> appLoader;
    std::shared_ptr<j2me::loader::JarLoader> libraryLoader;
    std::string mainClassName;
    bool isClass = false; // true if loading single class file
    std::optional<std::vector<uint8_t>> classData; // if single class
};

}
}
