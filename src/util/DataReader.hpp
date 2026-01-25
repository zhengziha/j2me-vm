#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace j2me {
namespace util {

class DataReader {
public:
    DataReader(const std::vector<uint8_t>& data) : data(data), pos(0), error(false) {}

    uint8_t readU1() {
        if (pos >= data.size()) {
            error = true;
            return 0;
        }
        return data[pos++];
    }

    uint16_t readU2() {
        if (pos + 2 > data.size()) {
            error = true;
            return 0;
        }
        uint16_t value = (data[pos] << 8) | data[pos + 1];
        pos += 2;
        return value;
    }

    uint32_t readU4() {
        if (pos + 4 > data.size()) {
            error = true;
            return 0;
        }
        uint32_t value = (data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | data[pos + 3];
        pos += 4;
        return value;
    }

    std::vector<uint8_t> readBytes(size_t length) {
        if (pos + length > data.size()) {
            error = true;
            return {};
        }
        std::vector<uint8_t> result(data.begin() + pos, data.begin() + pos + length);
        pos += length;
        return result;
    }
    
    bool hasMore() const {
        return pos < data.size();
    }
    
    size_t position() const {
        return pos;
    }

    size_t tell() const { return pos; }
    void seek(size_t p) { pos = p; }
    
    bool hasError() const { return error; }

private:
    const std::vector<uint8_t>& data;
    size_t pos;
    bool error;
};

} // namespace util
} // namespace j2me
