#include "HeapManager.hpp"
#include "../native/NativeInputStream.hpp"

namespace j2me {
namespace core {

JavaObject* HeapManager::allocate(std::shared_ptr<JavaClass> cls) {
    // 简单的对象分配 (存储在 vector 中)
    // Simple object allocation (stored in vector)
    objects.emplace_back(cls);
    return &objects.back();
}

void HeapManager::clear() {
    objects.clear();
    streams.clear();
}

int HeapManager::allocateStream(const uint8_t* data, size_t size) {
    // 分配原生输入流 (用于读取资源文件等)
    // Allocate native input stream (for reading resource files, etc.)
    auto stream = std::make_unique<natives::NativeInputStream>(data, size);
    int id = nextStreamId++;
    streams.push_back(std::move(stream));
    return id;
}

natives::NativeInputStream* HeapManager::getStream(int id) {
    if (id < 1 || id > (int)streams.size()) {
        return nullptr;
    }
    return streams[id - 1].get();
}

void HeapManager::removeStream(int id) {
    if (id < 1 || id > (int)streams.size()) {
        return;
    }
    // 释放流资源
    // Release stream resource
    streams[id - 1].reset();
}

} // namespace core
} // namespace j2me
