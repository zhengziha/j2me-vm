#pragma once

#include "RuntimeTypes.hpp"
#include <list>
#include <memory>

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

private:
    HeapManager() = default;
    
    // We store raw pointers in a list to own them. 
    // In a real GC, we'd need a more complex structure (e.g., arenas).
    // Using list to avoid pointer invalidation on resize.
    std::list<JavaObject> objects;
};

} // namespace core
} // namespace j2me
