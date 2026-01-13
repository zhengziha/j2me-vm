#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace j2me {
namespace core {

// Constant Pool Tags
enum ConstantTag : uint8_t {
    CONSTANT_Utf8 = 1,
    CONSTANT_Integer = 3,
    CONSTANT_Float = 4,
    CONSTANT_Long = 5,
    CONSTANT_Double = 6,
    CONSTANT_Class = 7,
    CONSTANT_String = 8,
    CONSTANT_Fieldref = 9,
    CONSTANT_Methodref = 10,
    CONSTANT_InterfaceMethodref = 11,
    CONSTANT_NameAndType = 12
};

struct ConstantPoolInfo {
    uint8_t tag;
    virtual ~ConstantPoolInfo() = default;
};

struct ConstantUtf8 : ConstantPoolInfo {
    std::string bytes;
};

struct ConstantInteger : ConstantPoolInfo {
    int32_t bytes;
};

struct ConstantFloat : ConstantPoolInfo {
    float bytes;
};

struct ConstantLong : ConstantPoolInfo {
    int64_t bytes;
};

struct ConstantDouble : ConstantPoolInfo {
    double bytes;
};

struct ConstantClass : ConstantPoolInfo {
    uint16_t name_index;
};

struct ConstantString : ConstantPoolInfo {
    uint16_t string_index;
};

struct ConstantRef : ConstantPoolInfo {
    uint16_t class_index;
    uint16_t name_and_type_index;
};

struct ConstantNameAndType : ConstantPoolInfo {
    uint16_t name_index;
    uint16_t descriptor_index;
};

struct AttributeInfo {
    uint16_t attribute_name_index;
    std::vector<uint8_t> info;
};

struct FieldInfo {
    uint16_t access_flags;
    uint16_t name_index;
    uint16_t descriptor_index;
    std::vector<AttributeInfo> attributes;
};

struct MethodInfo {
    uint16_t access_flags;
    uint16_t name_index;
    uint16_t descriptor_index;
    std::vector<AttributeInfo> attributes;
};

struct ClassFile {
    uint32_t magic;
    uint16_t minor_version;
    uint16_t major_version;
    std::vector<std::shared_ptr<ConstantPoolInfo>> constant_pool;
    uint16_t access_flags;
    uint16_t this_class;
    uint16_t super_class;
    std::vector<uint16_t> interfaces;
    std::vector<FieldInfo> fields;
    std::vector<MethodInfo> methods;
    std::vector<AttributeInfo> attributes;
};

} // namespace core
} // namespace j2me
