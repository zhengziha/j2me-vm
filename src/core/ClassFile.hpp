#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace j2me {
namespace core {

// Constant Pool Tags
// 常量池标签定义
enum ConstantTag : uint8_t {
    CONSTANT_Utf8 = 1,               // UTF-8 字符串
    CONSTANT_Integer = 3,            // 整型 (4字节)
    CONSTANT_Float = 4,              // 浮点型 (4字节)
    CONSTANT_Long = 5,               // 长整型 (8字节)
    CONSTANT_Double = 6,             // 双精度浮点型 (8字节)
    CONSTANT_Class = 7,              // 类或接口的符号引用
    CONSTANT_String = 8,             // 字符串类型字面量
    CONSTANT_Fieldref = 9,           // 字段的符号引用
    CONSTANT_Methodref = 10,         // 类中方法的符号引用
    CONSTANT_InterfaceMethodref = 11,// 接口中方法的符号引用
    CONSTANT_NameAndType = 12        // 字段或方法的部分符号引用
};

// 常量池项基类
struct ConstantPoolInfo {
    uint8_t tag; // 标签，标识常量类型
    virtual ~ConstantPoolInfo() = default;
};

// UTF-8 字符串常量
struct ConstantUtf8 : ConstantPoolInfo {
    std::string bytes; // 字符串内容
};

// 整型常量
struct ConstantInteger : ConstantPoolInfo {
    int32_t bytes; // 整型值 (大端序)
};

// 浮点型常量
struct ConstantFloat : ConstantPoolInfo {
    float bytes; // 浮点值 (IEEE 754)
};

// 长整型常量
struct ConstantLong : ConstantPoolInfo {
    int64_t bytes; // 长整型值
};

// 双精度浮点型常量
struct ConstantDouble : ConstantPoolInfo {
    double bytes; // 双精度值
};

// 类引用常量
struct ConstantClass : ConstantPoolInfo {
    uint16_t name_index; // 指向全限定类名的 ConstantUtf8 索引
};

// 字符串引用常量
struct ConstantString : ConstantPoolInfo {
    uint16_t string_index; // 指向字符串内容的 ConstantUtf8 索引
};

// 字段/方法引用常量基类
struct ConstantRef : ConstantPoolInfo {
    uint16_t class_index;         // 指向声明该字段/方法的类或接口的 ConstantClass 索引
    uint16_t name_and_type_index; // 指向字段/方法名称和类型的 ConstantNameAndType 索引
};

// 名称和类型常量
struct ConstantNameAndType : ConstantPoolInfo {
    uint16_t name_index;       // 指向字段/方法名称的 ConstantUtf8 索引
    uint16_t descriptor_index; // 指向字段/方法描述符的 ConstantUtf8 索引
};

// 属性信息结构
struct AttributeInfo {
    uint16_t attribute_name_index; // 属性名称索引
    std::vector<uint8_t> info;     // 属性数据
};

// 字段信息结构
struct FieldInfo {
    uint16_t access_flags;     // 访问标志 (public, private, static 等)
    uint16_t name_index;       // 字段名称索引
    uint16_t descriptor_index; // 字段描述符索引
    std::vector<AttributeInfo> attributes; // 字段属性表
};

// 方法信息结构
struct MethodInfo {
    uint16_t access_flags;     // 访问标志
    uint16_t name_index;       // 方法名称索引
    uint16_t descriptor_index; // 方法描述符索引
    std::vector<AttributeInfo> attributes; // 方法属性表 (如 Code 属性)
};

// 类文件结构 (映射 .class 文件格式)
struct ClassFile {
    uint32_t magic;           // 魔数 (0xCAFEBABE)
    uint16_t minor_version;   // 次版本号
    uint16_t major_version;   // 主版本号
    std::vector<std::shared_ptr<ConstantPoolInfo>> constant_pool; // 常量池
    uint16_t access_flags;    // 类访问标志
    uint16_t this_class;      // 当前类索引
    uint16_t super_class;     // 父类索引
    std::vector<uint16_t> interfaces; // 实现的接口索引列表
    std::vector<FieldInfo> fields;    // 字段表
    std::vector<MethodInfo> methods;  // 方法表
    std::vector<AttributeInfo> attributes; // 类属性表
};

} // namespace core
} // namespace j2me
