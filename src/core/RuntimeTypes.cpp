#include "RuntimeTypes.hpp"
#include <iostream>

namespace j2me {
namespace core {

JavaClass::JavaClass(std::shared_ptr<ClassFile> file) : rawFile(file) {
    // If rawFile is dummy (for Object), skip name extraction
    if (file->this_class == 0) return; 

    // Extract class name from constant pool
    auto classInfo = std::dynamic_pointer_cast<ConstantClass>(file->constant_pool[file->this_class]);
    if (!classInfo) {
        std::cerr << "Invalid this_class index" << std::endl;
        return;
    }
    auto nameInfo = std::dynamic_pointer_cast<ConstantUtf8>(file->constant_pool[classInfo->name_index]);
    name = nameInfo->bytes;
}

void JavaClass::link(std::shared_ptr<JavaClass> parent) {
    superClass = parent;
    size_t offset = 0;
    
    if (parent) {
        offset = parent->instanceSize;
        // Copy parent field offsets? Not strictly needed if we look up recursively or flatten.
        // For simplicity, we'll just track our own fields starting from parent's end.
        // But for fast lookup, we might want a flattened map.
        fieldOffsets = parent->fieldOffsets; 
    }

    for (const auto& field : rawFile->fields) {
        // Skip static fields for now (they belong to class, not instance)
        if (field.access_flags & 0x0008) continue; 

        auto nameInfo = std::dynamic_pointer_cast<ConstantUtf8>(rawFile->constant_pool[field.name_index]);
        fieldOffsets[nameInfo->bytes] = offset++;
    }
    instanceSize = offset;
}

JavaObject::JavaObject(std::shared_ptr<JavaClass> cls) : cls(cls) {
    fields.resize(cls->instanceSize);
}

} // namespace core
} // namespace j2me
