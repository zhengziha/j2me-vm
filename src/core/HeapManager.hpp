#pragma once

#include "RuntimeTypes.hpp"
#include "../native/NativeInputStream.hpp"
#include <list>
#include <memory>
#include <vector>
#include <cstdint>

namespace j2me {
namespace core {

class HeapManager {
public:
    // Singleton for simplicity in Phase 2
    static HeapManager& getInstance() {
        static HeapManager instance;
        return instance;
    }

    JavaObject* allocate(std::shared_ptr<JavaClass> cls);
    
    // Very basic "GC" - just clear everything (for shutdown)
    void clear();
    
    // Stream management for NativeInputStream
    int allocateStream(const uint8_t* data, size_t size);
    int allocateStreamWithPath(const uint8_t* data, size_t size, const std::string& path);
    natives::NativeInputStream* getStream(int id);
    void removeStream(int id);

private:
    HeapManager() = default;
    
    // We store raw pointers in a list to own them. 
    // In a real GC, we'd need a more complex structure (e.g., arenas).
    // Using list to avoid pointer invalidation on resize.
    std::list<JavaObject> objects;
    
    // Stream storage
    std::vector<std::unique_ptr<j2me::natives::NativeInputStream>> streams;
    int nextStreamId = 1;
};

} // namespace core
} // namespace j2me
