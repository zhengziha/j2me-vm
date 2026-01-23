#include "ClassParser.hpp"
#include "Logger.hpp"
#include <iostream>

namespace j2me {
namespace core {

std::shared_ptr<ClassFile> ClassParser::parse(const std::vector<uint8_t>& data) {
    util::DataReader reader(data);
    auto classFile = std::make_shared<ClassFile>();

    // 读取魔数 (Magic Number)
    // Read Magic Number
    classFile->magic = reader.readU4();
    if (classFile->magic != 0xCAFEBABE) {
        throw std::runtime_error("Invalid class file magic");
    }

    // 读取版本号
    // Read Version
    classFile->minor_version = reader.readU2();
    classFile->major_version = reader.readU2();

    // 解析常量池
    // Parse Constant Pool
    parseConstantPool(reader, *classFile);

    // classFile->access_flags = reader.readU2();
    // classFile->this_class = reader.readU2();
    // classFile->super_class = reader.readU2();
    
    // 修复: 访问标志、当前类、父类索引位于常量池之后
    // Fix: Access flags, this_class, super_class are AFTER constant pool
    classFile->access_flags = reader.readU2();
    classFile->this_class = reader.readU2();
    classFile->super_class = reader.readU2();

    // 解析接口、字段、方法和属性
    // Parse Interfaces, Fields, Methods, Attributes
    parseInterfaces(reader, *classFile);
    parseFields(reader, *classFile);
    parseMethods(reader, *classFile);
    parseAttributes(reader, classFile->attributes);

    return classFile;
}

void ClassParser::parseConstantPool(util::DataReader& reader, ClassFile& classFile) {
    uint16_t cp_count = reader.readU2();
    // 常量池索引从 1 开始，所以调整大小为 count 并忽略索引 0
    // Constant pool is 1-indexed, so we resize to count and ignore index 0
    classFile.constant_pool.resize(cp_count);

    for (int i = 1; i < cp_count; ++i) {
        uint8_t tag = reader.readU1();
        std::shared_ptr<ConstantPoolInfo> info;

        switch (tag) {
            case CONSTANT_Utf8: {
                // UTF-8 字符串常量
                auto utf8 = std::make_shared<ConstantUtf8>();
                utf8->tag = tag;
                uint16_t length = reader.readU2();
                auto bytes = reader.readBytes(length);
                utf8->bytes = std::string(bytes.begin(), bytes.end());
                info = utf8;
                break;
            }
            case CONSTANT_Integer: {
                // 整型常量
                auto integer = std::make_shared<ConstantInteger>();
                integer->tag = tag;
                integer->bytes = static_cast<int32_t>(reader.readU4());
                info = integer;
                break;
            }
            case CONSTANT_Float: {
                // 浮点型常量
                auto flt = std::make_shared<ConstantFloat>();
                flt->tag = tag;
                uint32_t bytes = reader.readU4();
                flt->bytes = *reinterpret_cast<float*>(&bytes);
                info = flt;
                break;
            }
            case CONSTANT_Long: {
                // 长整型常量 (占用两个槽位)
                auto lng = std::make_shared<ConstantLong>();
                lng->tag = tag;
                uint32_t high = reader.readU4();
                uint32_t low = reader.readU4();
                lng->bytes = (static_cast<int64_t>(high) << 32) | low;
                info = lng;
                break;
            }
            case CONSTANT_Double: {
                // 双精度浮点型常量 (占用两个槽位)
                auto dbl = std::make_shared<ConstantDouble>();
                dbl->tag = tag;
                uint32_t high = reader.readU4();
                uint32_t low = reader.readU4();
                int64_t bits = (static_cast<int64_t>(high) << 32) | low;
                dbl->bytes = *reinterpret_cast<double*>(&bits);
                info = dbl;
                break;
            }
            case CONSTANT_Class: {
                // 类引用常量
                auto cls = std::make_shared<ConstantClass>();
                cls->tag = tag;
                cls->name_index = reader.readU2();
                info = cls;
                break;
            }
            case CONSTANT_String: {
                // 字符串引用常量
                auto str = std::make_shared<ConstantString>();
                str->tag = tag;
                str->string_index = reader.readU2();
                info = str;
                break;
            }
            case CONSTANT_Fieldref:
            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref: {
                // 字段/方法引用常量
                auto ref = std::make_shared<ConstantRef>();
                ref->tag = tag;
                ref->class_index = reader.readU2();
                ref->name_and_type_index = reader.readU2();
                info = ref;
                break;
            }
            case CONSTANT_NameAndType: {
                // 名称和类型常量
                auto nt = std::make_shared<ConstantNameAndType>();
                nt->tag = tag;
                nt->name_index = reader.readU2();
                nt->descriptor_index = reader.readU2();
                info = nt;
                break;
            }
            default:
                throw std::runtime_error("Unknown constant pool tag: " + std::to_string(tag));
        }
        if (info) {
             classFile.constant_pool[i] = info;
             if (tag == CONSTANT_Long || tag == CONSTANT_Double) {
                 i++; // Skip next slot
             }
        }
    }
}

void ClassParser::parseInterfaces(util::DataReader& reader, ClassFile& classFile) {
    uint16_t count = reader.readU2();
    for (int i = 0; i < count; ++i) {
        // 读取接口索引
        classFile.interfaces.push_back(reader.readU2());
    }
}

void ClassParser::parseFields(util::DataReader& reader, ClassFile& classFile) {
    uint16_t count = reader.readU2();
    LOG_DEBUG("[ClassParser::parseFields] Parsing " + std::to_string(count) + " fields");
    for (int i = 0; i < count; ++i) {
        FieldInfo field;
        // 读取访问标志、名称索引、描述符索引
        field.access_flags = reader.readU2();
        field.name_index = reader.readU2();
        field.descriptor_index = reader.readU2();
        // 解析字段属性
        parseAttributes(reader, field.attributes);
        classFile.fields.push_back(field);
        
        auto nameInfo = std::dynamic_pointer_cast<ConstantUtf8>(classFile.constant_pool[field.name_index]);
        if (nameInfo) {
            LOG_DEBUG("[ClassParser::parseFields]   Field " + std::to_string(i) + ": " + nameInfo->bytes + " access_flags=" + std::to_string(field.access_flags));
        } else {
            LOG_DEBUG("[ClassParser::parseFields]   Field " + std::to_string(i) + ": name_index=" + std::to_string(field.name_index) + " (null nameInfo)");
        }
    }
}

void ClassParser::parseMethods(util::DataReader& reader, ClassFile& classFile) {
    uint16_t count = reader.readU2();
    for (int i = 0; i < count; ++i) {
        MethodInfo method;
        // 读取访问标志、名称索引、描述符索引
        method.access_flags = reader.readU2();
        method.name_index = reader.readU2();
        method.descriptor_index = reader.readU2();
        // 解析方法属性 (如 Code 属性)
        parseAttributes(reader, method.attributes);
        classFile.methods.push_back(method);
    }
}

void ClassParser::parseAttributes(util::DataReader& reader, std::vector<AttributeInfo>& attributes) {
    uint16_t count = reader.readU2();
    for (int i = 0; i < count; ++i) {
        AttributeInfo attr;
        // 读取属性名称索引和长度
        attr.attribute_name_index = reader.readU2();
        uint32_t length = reader.readU4();
        // 读取属性内容
        attr.info = reader.readBytes(length);
        attributes.push_back(attr);
    }
}

} // namespace core
} // namespace j2me
