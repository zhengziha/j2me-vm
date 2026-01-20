#include "RuntimeTypes.hpp"
#include "Logger.hpp"
#include <iostream>

namespace j2me {
namespace core {

JavaClass::JavaClass(std::shared_ptr<ClassFile> file) : rawFile(file) {
    // If rawFile is dummy (for Object), skip name extraction
    if (file->this_class == 0) return; 

    // Extract class name from constant pool
    auto classInfo = std::dynamic_pointer_cast<ConstantClass>(file->constant_pool[file->this_class]);
    if (!classInfo) {
        LOG_ERROR("Invalid this_class index");
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
        fieldOffsets = parent->fieldOffsets; 
    }

    LOG_DEBUG("[JavaClass::link] Linking class: " + name + " with " + std::to_string(rawFile->fields.size()) + " fields");
    
    for (const auto& field : rawFile->fields) {
        auto nameInfo = std::dynamic_pointer_cast<ConstantUtf8>(rawFile->constant_pool[field.name_index]);
        auto descInfo = std::dynamic_pointer_cast<ConstantUtf8>(rawFile->constant_pool[field.descriptor_index]);
        
        LOG_DEBUG("[JavaClass::link]   Field: " + nameInfo->bytes + " desc=" + descInfo->bytes + " access_flags=" + std::to_string(field.access_flags));
        
        if (field.access_flags & 0x0008) {
            staticFields[nameInfo->bytes] = 0;
        } else {
            fieldOffsets[nameInfo->bytes] = offset++;
        }
    }
    instanceSize = offset;
    LOG_DEBUG("[JavaClass::link]   instanceSize=" + std::to_string(instanceSize) + " fieldOffsets size=" + std::to_string(fieldOffsets.size()));
}

JavaObject::JavaObject(std::shared_ptr<JavaClass> cls) : cls(cls) {
    if (cls) {
        fields.resize(cls->instanceSize);
    }
}

} // namespace core
} // namespace j2me
