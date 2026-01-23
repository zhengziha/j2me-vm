#include "JarLoader.hpp"
#include "../core/Logger.hpp"
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
    // 打开 ZIP 归档 (只读模式)
    // Open ZIP archive (Read Only)
    archive = zip_open(path.c_str(), ZIP_RDONLY, &error);
    if (!archive) {
        LOG_ERROR("Failed to open JAR: " + path + " Error code: " + std::to_string(error));
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
    // 查找文件索引
    zip_int64_t index = zip_name_locate(archive, filename.c_str(), 0);
    return index >= 0;
}

std::optional<std::vector<uint8_t>> JarLoader::getFile(const std::string& filename) {
    if (!archive) return std::nullopt;

    zip_int64_t index = zip_name_locate(archive, filename.c_str(), 0);
    if (index < 0) return std::nullopt;

    // 获取文件状态 (大小等)
    struct zip_stat st;
    if (zip_stat_index(archive, index, 0, &st) != 0) return std::nullopt;

    // 打开文件
    zip_file_t* file = zip_fopen_index(archive, index, 0);
    if (!file) return std::nullopt;

    // 读取内容
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
