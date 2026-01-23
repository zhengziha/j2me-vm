#pragma once

#include <vector>
#include <cstdint>

namespace j2me {
namespace natives {

class NativeInputStream {
public:
    NativeInputStream(const uint8_t* data, size_t size);
    
    int read();
    int read(uint8_t* buffer, int len);
    long skip(long n);
    int available();
    void close();
    
    const uint8_t* getData() const { return data.data(); }
    size_t getSize() const { return data.size(); }
    size_t getPosition() const { return position; }
    
private:
    std::vector<uint8_t> data;
    size_t position;
};

} // namespace natives
} // namespace j2me
