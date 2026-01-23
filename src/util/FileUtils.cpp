#include "FileUtils.hpp"
#include <fstream>

namespace j2me {
namespace util {

bool FileUtils::isClassFile(const std::string& path) {
    return path.size() >= 6 && path.substr(path.size() - 6) == ".class";
}

std::optional<std::vector<uint8_t>> FileUtils::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(size);
    if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
        return std::nullopt;
    }
    
    return data;
}

}
}
