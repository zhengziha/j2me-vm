#include "NativeInputStream.hpp"
#include <cstring>
#include <algorithm>
#include <iostream>
#include <iomanip>

namespace j2me {
namespace natives {

NativeInputStream::NativeInputStream(const uint8_t* dataPtr, size_t size) 
    : data(dataPtr, dataPtr + size), position(0), filePath("unknown") {
    // std::cout << "[NativeInputStream] Created with size: " << size << " pos: " << position << " id: " << this << " path: " << filePath << std::endl;
    if (size > 0) {
        // printf("[NativeInputStream] First 16 bytes: ");
        for (size_t i = 0; i < std::min((size_t)16, size); i++) {
            // printf("%02X ", data[i]);
        }
        // printf("\n");
    }
}

int NativeInputStream::read() {
    // std::cout << "[NativeInputStream] read() BEFORE - id: " << this << " path: " << filePath << " size: " << data.size() << " position: " << position << " available: " << (data.size() - position) << std::endl;
    
    if (position >= data.size()) {
        // std::cout << "[NativeInputStream] read() AFTER - id: " << this << " path: " << filePath << " returning: -1 (EOF)" << std::endl;
        return -1;
    }
    int byte = data[position++];
    // std::cout << "[NativeInputStream] read() AFTER - id: " << this << " path: " << filePath << " size: " << data.size() << " position: " << position << " available: " << (data.size() - position) << " returning: " << byte << std::endl;
    return byte;
}

int NativeInputStream::read(uint8_t* buffer, int len) {
    // std::cout << "[NativeInputStream] read(buffer, " << len << ") BEFORE - id: " << this << " path: " << filePath << " size: " << data.size() << " position: " << position << " available: " << (data.size() - position) << std::endl;
    
    if (position >= data.size()) {
        // std::cout << "[NativeInputStream] read(buffer, " << len << ") AFTER - id: " << this << " path: " << filePath << " returning: -1 (EOF)" << std::endl;
        return -1;
    }
    
    size_t remaining = data.size() - position;
    size_t toRead = (size_t)len < remaining ? (size_t)len : remaining;
    
    memcpy(buffer, data.data() + position, toRead);
    position += toRead;
    
    // std::cout << "[NativeInputStream] read(buffer, " << len << ") AFTER - id: " << this << " path: " << filePath << " size: " << data.size() << " position: " << position << " available: " << (data.size() - position) << " returning: " << (int)toRead << std::endl;
    return (int)toRead;
}

long NativeInputStream::skip(long n) {
    // std::cout << "[NativeInputStream] skip(" << n << ") BEFORE - id: " << this << " path: " << filePath << " size: " << data.size() << " position: " << position << " available: " << (data.size() - position) << std::endl;
    
    if (n <= 0) {
        // std::cout << "[NativeInputStream] skip(" << n << ") AFTER - id: " << this << " path: " << filePath << " returning: 0 (n <= 0)" << std::endl;
        return 0;
    }
    if (position >= data.size()) {
        // std::cout << "[NativeInputStream] skip(" << n << ") AFTER - id: " << this << " path: " << filePath << " returning: 0 (EOF)" << std::endl;
        return 0;
    }
    
    size_t remaining = data.size() - position;
    size_t toSkip = (size_t)n < remaining ? (size_t)n : remaining;
    
    position += toSkip;
    
    // std::cout << "[NativeInputStream] skip(" << n << ") AFTER - id: " << this << " path: " << filePath << " size: " << data.size() << " position: " << position << " available: " << (data.size() - position) << " returning: " << (long)toSkip << std::endl;
    return (long)toSkip;
}

void NativeInputStream::seek(long pos) {
    // std::cout << "[NativeInputStream] seek(" << pos << ") BEFORE - id: " << this << " path: " << filePath << " size: " << data.size() << " position: " << position << " available: " << (data.size() - position) << std::endl;
    
    if (pos < 0) {
        position = 0;
    } else if (pos >= (long)data.size()) {
        position = data.size();
    } else {
        position = (size_t)pos;
    }
    
    // std::cout << "[NativeInputStream] seek(" << pos << ") AFTER - id: " << this << " path: " << filePath << " size: " << data.size() << " position: " << position << " available: " << (data.size() - position) << std::endl;
}

int NativeInputStream::available() {
    int available_bytes = (position >= data.size()) ? 0 : (int)(data.size() - position);
    // std::cout << "[NativeInputStream] available() - id: " << this << " path: " << filePath << " size: " << data.size() << " position: " << position << " available: " << available_bytes << std::endl;
    return available_bytes;
}

void NativeInputStream::close() {
}

void NativeInputStream::mark(int readlimit) {
    // std::cout << "[NativeInputStream] mark(" << readlimit << ") - id: " << this << " path: " << filePath << " size: " << data.size() << " position: " << position << " markPosition: " << markPosition << std::endl;
    // For a memory stream, readlimit is irrelevant, we can always reset.
    markPosition = position;
    // std::cout << "[NativeInputStream] mark(" << readlimit << ") AFTER - id: " << this << " path: " << filePath << " new markPosition: " << markPosition << std::endl;
}

void NativeInputStream::reset() {
    // std::cout << "[NativeInputStream] reset() BEFORE - id: " << this << " path: " << filePath << " size: " << data.size() << " position: " << position << " markPosition: " << markPosition << std::endl;
    position = markPosition;
    // std::cout << "[NativeInputStream] reset() AFTER - id: " << this << " path: " << filePath << " size: " << data.size() << " position: " << position << " available: " << (data.size() - position) << std::endl;
}

} // namespace natives
} // namespace j2me
