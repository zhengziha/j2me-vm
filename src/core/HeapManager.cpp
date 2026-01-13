#include "HeapManager.hpp"

namespace j2me {
namespace core {

JavaObject* HeapManager::allocate(std::shared_ptr<JavaClass> cls) {
    objects.emplace_back(cls);
    return &objects.back();
}

void HeapManager::clear() {
    objects.clear();
}

} // namespace core
} // namespace j2me
