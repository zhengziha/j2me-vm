#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "ClassFile.hpp"

namespace j2me {
namespace core {

class JavaObject;

// Runtime representation of a Class
class JavaClass {
public:
    std::shared_ptr<ClassFile> rawFile;
    std::string name;
    std::shared_ptr<JavaClass> superClass;
    
    // Field layout map: name -> offset in object fields vector
    std::map<std::string, size_t> fieldOffsets;
    size_t instanceSize = 0; // Number of fields (slots)
    
    // Static fields storage: name -> value
    std::map<std::string, int64_t> staticFields;

    JavaClass(std::shared_ptr<ClassFile> file);
    
    // Resolve hierarchy and calculate field offsets
    void link(std::shared_ptr<JavaClass> parent);
};

// Runtime representation of an Object instance
class JavaObject {
public:
    std::shared_ptr<JavaClass> cls;
    std::vector<int64_t> fields; // Store all fields as 64-bit slots for simplicity

    JavaObject(std::shared_ptr<JavaClass> cls);
};

} // namespace core
} // namespace j2me
