#pragma once

#include "ClassFile.hpp"
#include "../util/DataReader.hpp"
#include <memory>
#include <vector>

namespace j2me {
namespace core {

class ClassParser {
public:
    std::shared_ptr<ClassFile> parse(const std::vector<uint8_t>& data);

private:
    void parseConstantPool(util::DataReader& reader, ClassFile& classFile);
    void parseInterfaces(util::DataReader& reader, ClassFile& classFile);
    void parseFields(util::DataReader& reader, ClassFile& classFile);
    void parseMethods(util::DataReader& reader, ClassFile& classFile);
    void parseAttributes(util::DataReader& reader, std::vector<AttributeInfo>& attributes);
};

} // namespace core
} // namespace j2me
