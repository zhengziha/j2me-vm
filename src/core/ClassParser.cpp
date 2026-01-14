#include "ClassParser.hpp"
#include "Logger.hpp"
#include <iostream>

namespace j2me {
namespace core {

std::shared_ptr<ClassFile> ClassParser::parse(const std::vector<uint8_t>& data) {
    util::DataReader reader(data);
    auto classFile = std::make_shared<ClassFile>();

    classFile->magic = reader.readU4();
    if (classFile->magic != 0xCAFEBABE) {
        throw std::runtime_error("Invalid class file magic");
    }

    classFile->minor_version = reader.readU2();
    classFile->major_version = reader.readU2();

    parseConstantPool(reader, *classFile);

    // classFile->access_flags = reader.readU2();
    // classFile->this_class = reader.readU2();
    // classFile->super_class = reader.readU2();
    
    // Fix: Access flags, this_class, super_class are AFTER constant pool
    classFile->access_flags = reader.readU2();
    classFile->this_class = reader.readU2();
    classFile->super_class = reader.readU2();

    parseInterfaces(reader, *classFile);
    parseFields(reader, *classFile);
    parseMethods(reader, *classFile);
    parseAttributes(reader, classFile->attributes);

    return classFile;
}

void ClassParser::parseConstantPool(util::DataReader& reader, ClassFile& classFile) {
    uint16_t cp_count = reader.readU2();
    // Constant pool is 1-indexed, so we resize to count and ignore index 0
    classFile.constant_pool.resize(cp_count);

    for (int i = 1; i < cp_count; ++i) {
        uint8_t tag = reader.readU1();
        std::shared_ptr<ConstantPoolInfo> info;

        switch (tag) {
            case CONSTANT_Utf8: {
                auto utf8 = std::make_shared<ConstantUtf8>();
                utf8->tag = tag;
                uint16_t length = reader.readU2();
                auto bytes = reader.readBytes(length);
                utf8->bytes = std::string(bytes.begin(), bytes.end());
                info = utf8;
                break;
            }
            case CONSTANT_Integer: {
                auto integer = std::make_shared<ConstantInteger>();
                integer->tag = tag;
                integer->bytes = static_cast<int32_t>(reader.readU4());
                info = integer;
                break;
            }
            case CONSTANT_Float: {
                auto flt = std::make_shared<ConstantFloat>();
                flt->tag = tag;
                uint32_t bytes = reader.readU4();
                flt->bytes = *reinterpret_cast<float*>(&bytes);
                info = flt;
                break;
            }
            case CONSTANT_Long: {
                auto lng = std::make_shared<ConstantLong>();
                lng->tag = tag;
                uint32_t high = reader.readU4();
                uint32_t low = reader.readU4();
                lng->bytes = (static_cast<int64_t>(high) << 32) | low;
                info = lng;
                break;
            }
            case CONSTANT_Double: {
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
                auto cls = std::make_shared<ConstantClass>();
                cls->tag = tag;
                cls->name_index = reader.readU2();
                info = cls;
                break;
            }
            case CONSTANT_String: {
                auto str = std::make_shared<ConstantString>();
                str->tag = tag;
                str->string_index = reader.readU2();
                info = str;
                break;
            }
            case CONSTANT_Fieldref:
            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref: {
                auto ref = std::make_shared<ConstantRef>();
                ref->tag = tag;
                ref->class_index = reader.readU2();
                ref->name_and_type_index = reader.readU2();
                info = ref;
                break;
            }
            case CONSTANT_NameAndType: {
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
        classFile.interfaces.push_back(reader.readU2());
    }
}

void ClassParser::parseFields(util::DataReader& reader, ClassFile& classFile) {
    uint16_t count = reader.readU2();
    LOG_DEBUG("[ClassParser::parseFields] Parsing " + std::to_string(count) + " fields");
    for (int i = 0; i < count; ++i) {
        FieldInfo field;
        field.access_flags = reader.readU2();
        field.name_index = reader.readU2();
        field.descriptor_index = reader.readU2();
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
        method.access_flags = reader.readU2();
        method.name_index = reader.readU2();
        method.descriptor_index = reader.readU2();
        parseAttributes(reader, method.attributes);
        classFile.methods.push_back(method);
    }
}

void ClassParser::parseAttributes(util::DataReader& reader, std::vector<AttributeInfo>& attributes) {
    uint16_t count = reader.readU2();
    for (int i = 0; i < count; ++i) {
        AttributeInfo attr;
        attr.attribute_name_index = reader.readU2();
        uint32_t length = reader.readU4();
        attr.info = reader.readBytes(length);
        attributes.push_back(attr);
    }
}

} // namespace core
} // namespace j2me
