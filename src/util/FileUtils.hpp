#pragma once

#include <string>
#include <vector>
#include <optional>

namespace j2me {
namespace util {

class FileUtils {
public:
    static bool isClassFile(const std::string& path);
    static std::optional<std::vector<uint8_t>> readFile(const std::string& path);
};

}
}
