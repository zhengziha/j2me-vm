#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace j2me {
namespace natives {

class NativeInputStream {
public:
    NativeInputStream(const uint8_t* data, size_t size);
    
    int read();
    int read(uint8_t* buffer, int len);
    long skip(long n);
    void seek(long pos);
    int available();
    void close();
    
    // Mark/Reset support
    void mark(int readlimit);
    void reset();
    bool markSupported() const { return true; }

    const uint8_t* getData() const { return data.data(); }
    size_t getSize() const { return data.size(); }
    size_t getPosition() const { return position; }
    
    // Path management
    void setFilePath(const std::string& path) { filePath = path; }
    const std::string& getFilePath() const { return filePath; }
    
private:
    std::vector<uint8_t> data;
    size_t position;
    size_t markPosition = 0;
    std::string filePath;
};

} // namespace natives
} // namespace j2me
