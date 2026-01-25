#include "JarLoader.hpp"
#include "../core/Logger.hpp"
#include <iostream>

namespace j2me {
namespace loader {

JarLoader::JarLoader() : archive(nullptr) {}

JarLoader::~JarLoader() {
    close();
}

void JarLoader::buildFileMap() {
    if (!archive) return;

    fileMap.clear();
    int err = unzGoToFirstFile(archive);
    if (err != UNZ_OK) return;

    do {
        unz_file_info info;
        char filename[256];
        err = unzGetCurrentFileInfo(archive, &info, filename, sizeof(filename), nullptr, 0, nullptr, 0);
        if (err == UNZ_OK) {
            unz_file_pos pos;
            if (unzGetFilePos(archive, &pos) == UNZ_OK) {
                fileMap[filename] = pos;
            }
        }
        err = unzGoToNextFile(archive);
    } while (err == UNZ_OK);
}

bool JarLoader::load(const std::string& path) {
    close();
    // 打开 ZIP 归档 (只读模式)
    // Open ZIP archive (Read Only)
    archive = unzOpen(path.c_str());
    if (!archive) {
        LOG_ERROR("Failed to open JAR: " + path);
        return false;
    }
    jarPath = path;
    buildFileMap();
    return true;
}

void JarLoader::close() {
    if (archive) {
        unzClose(archive);
        archive = nullptr;
        fileMap.clear();
    }
}

bool JarLoader::hasFile(const std::string& filename) {
    if (!archive) return false;
    return fileMap.find(filename) != fileMap.end();
}

std::optional<std::vector<uint8_t>> JarLoader::getFile(const std::string& filename) {
    if (!archive) return std::nullopt;

    auto it = fileMap.find(filename);
    if (it == fileMap.end()) return std::nullopt;

    // 定位到文件
    if (unzGoToFilePos(archive, &(it->second)) != UNZ_OK) return std::nullopt;

    // 获取文件信息
    unz_file_info info;
    if (unzGetCurrentFileInfo(archive, &info, nullptr, 0, nullptr, 0, nullptr, 0) != UNZ_OK) return std::nullopt;

    // 打开文件
    if (unzOpenCurrentFile(archive) != UNZ_OK) return std::nullopt;

    // 读取内容
    std::vector<uint8_t> buffer(info.uncompressed_size);
    int bytesRead = unzReadCurrentFile(archive, buffer.data(), buffer.size());
    unzCloseCurrentFile(archive);

    if (bytesRead != (int)info.uncompressed_size) {
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
