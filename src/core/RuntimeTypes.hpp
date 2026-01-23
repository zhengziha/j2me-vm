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
// 类的运行时表示
class JavaClass {
public:
    std::shared_ptr<ClassFile> rawFile; // 原始 .class 文件结构
    std::string name;                   // 类名 (例如 java/lang/String)
    std::shared_ptr<JavaClass> superClass; // 父类引用
    std::vector<std::shared_ptr<JavaClass>> interfaces; // 实现的接口列表
    
    // Field layout map: name -> offset in object fields vector
    // 字段布局映射: 字段名|描述符 -> 对象实例字段数组中的索引 (Offset)
    // 用于快速查找实例字段的位置
    std::map<std::string, size_t> fieldOffsets;
    size_t instanceSize = 0; // Number of fields (slots) // 实例字段总数 (对象所需槽位数)
    
    // Static fields storage: name -> value
    // 静态字段存储: 字段名|描述符 -> 值 (int64_t 存储所有基本类型和引用)
    std::map<std::string, int64_t> staticFields;

    // Flag to track if static initialization has been done
    // 标志: 类是否已初始化 (<clinit> 是否已执行)
    bool initialized = false;
    
    // Flag to track if class is currently being initialized (to prevent cycles)
    // 标志: 类是否正在初始化中 (用于防止循环初始化死锁)
    bool initializing = false;

    JavaClass(std::shared_ptr<ClassFile> file);
    
    // Resolve hierarchy and calculate field offsets
    // 链接类: 解析继承层次结构并计算字段偏移量
    void link(std::shared_ptr<JavaClass> parent);
};

// Runtime representation of an Object instance
// 对象实例的运行时表示
class JavaObject {
public:
    std::shared_ptr<JavaClass> cls; // 所属类的引用
    std::vector<int64_t> fields; // Store all fields as 64-bit slots for simplicity
                                 // 实例字段存储: 使用 int64_t 数组存储所有字段值 (包括引用和基本类型)
                                 // 索引对应于 JavaClass::fieldOffsets 中的值

    JavaObject(std::shared_ptr<JavaClass> cls);
};

} // namespace core
} // namespace j2me
