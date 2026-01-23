#include "NativeInputStream.hpp"
#include <cstring>
#include <algorithm>

namespace j2me {
namespace natives {

NativeInputStream::NativeInputStream(const uint8_t* dataPtr, size_t size) 
    : data(dataPtr, dataPtr + size), position(0) {
}

int NativeInputStream::read() {
    if (position >= data.size()) {
        return -1;
    }
    return data[position++];
}

int NativeInputStream::read(uint8_t* buffer, int len) {
    if (position >= data.size()) {
        return -1;
    }
    
    size_t remaining = data.size() - position;
    size_t toRead = (size_t)len < remaining ? (size_t)len : remaining;
    
    memcpy(buffer, data.data() + position, toRead);
    position += toRead;
    
    return (int)toRead;
}

long NativeInputStream::skip(long n) {
    if (n <= 0) return 0;
    if (position >= data.size()) return 0;
    
    size_t remaining = data.size() - position;
    size_t toSkip = (size_t)n < remaining ? (size_t)n : remaining;
    
    position += toSkip;
    return (long)toSkip;
}

int NativeInputStream::available() {
    if (position >= data.size()) return 0;
    return (int)(data.size() - position);
}

void NativeInputStream::close() {
}

} // namespace natives
} // namespace j2me
