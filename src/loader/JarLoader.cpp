#include "JarLoader.hpp"
#include <iostream>

namespace j2me {
namespace loader {

JarLoader::JarLoader() : archive(nullptr) {}

JarLoader::~JarLoader() {
    close();
}

bool JarLoader::load(const std::string& path) {
    close();
    int error = 0;
    archive = zip_open(path.c_str(), ZIP_RDONLY, &error);
    if (!archive) {
        std::cerr << "Failed to open JAR: " << path << " Error code: " << error << std::endl;
        return false;
    }
    jarPath = path;
    return true;
}

void JarLoader::close() {
    if (archive) {
        zip_close(archive);
        archive = nullptr;
    }
}

bool JarLoader::hasFile(const std::string& filename) {
    if (!archive) return false;
    zip_int64_t index = zip_name_locate(archive, filename.c_str(), 0);
    return index >= 0;
}

std::optional<std::vector<uint8_t>> JarLoader::getFile(const std::string& filename) {
    if (!archive) return std::nullopt;

    zip_int64_t index = zip_name_locate(archive, filename.c_str(), 0);
    if (index < 0) return std::nullopt;

    struct zip_stat st;
    if (zip_stat_index(archive, index, 0, &st) != 0) return std::nullopt;

    zip_file_t* file = zip_fopen_index(archive, index, 0);
    if (!file) return std::nullopt;

    std::vector<uint8_t> buffer(st.size);
    zip_int64_t bytesRead = zip_fread(file, buffer.data(), st.size);
    zip_fclose(file);

    if (bytesRead != (zip_int64_t)st.size) {
        return std::nullopt;
    }

    return buffer;
}

std::optional<std::string> JarLoader::getManifest() {
    auto data = getFile("META-INF/MANIFEST.MF");
    if (!data) return std::nullopt;
    return std::string(data->begin(), data->end());
}

} // namespace loader
} // namespace j2me
