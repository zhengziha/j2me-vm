#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <minizip/unzip.h>
#include <cstdint>

namespace j2me {
namespace loader {

class JarLoader {
public:
    JarLoader();
    ~JarLoader();

    // Load a JAR file
    bool load(const std::string& path);

    // Get file content from JAR
    std::optional<std::vector<uint8_t>> getFile(const std::string& filename);

    // Check if file exists
    bool hasFile(const std::string& filename);
    
    // Get Manifest content as string
    std::optional<std::string> getManifest();

    // Close the JAR
    void close();

private:
    unzFile archive = nullptr;
    std::string jarPath;
    std::map<std::string, unz_file_pos> fileMap;
    void buildFileMap();
};

} // namespace loader
} // namespace j2me
