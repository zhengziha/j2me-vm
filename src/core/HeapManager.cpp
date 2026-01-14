#include "HeapManager.hpp"
#include "../native/NativeInputStream.hpp"

namespace j2me {
namespace core {

JavaObject* HeapManager::allocate(std::shared_ptr<JavaClass> cls) {
    objects.emplace_back(cls);
    return &objects.back();
}

void HeapManager::clear() {
    objects.clear();
    streams.clear();
}

int HeapManager::allocateStream(const uint8_t* data, size_t size) {
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
    streams[id - 1].reset();
}

} // namespace core
} // namespace j2me
