#include "NativeInputStream.hpp"
#include <cstring>

namespace j2me {
namespace natives {

NativeInputStream::NativeInputStream(const uint8_t* data, size_t size) 
    : data(data), size(size), position(0) {
}

int NativeInputStream::read() {
    if (position >= size) {
        return -1;
    }
    return data[position++];
}

int NativeInputStream::read(uint8_t* buffer, int len) {
    if (position >= size) {
        return -1;
    }
    
    size_t remaining = size - position;
    size_t toRead = (size_t)len < remaining ? (size_t)len : remaining;
    
    memcpy(buffer, data + position, toRead);
    position += toRead;
    
    return (int)toRead;
}

void NativeInputStream::close() {
}

} // namespace natives
} // namespace j2me
