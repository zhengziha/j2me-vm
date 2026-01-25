#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

namespace j2me {
namespace core {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    ERROR = 2,
    NONE = 3
};

class Logger {
public:
    static Logger& getInstance();

    void setLevel(LogLevel level);
    LogLevel getLevel() const;

    void debug(const std::string& message);
    void info(const std::string& message);
    void error(const std::string& message);

    template<typename... Args>
    void debug(const std::string& format, Args... args);

    template<typename... Args>
    void info(const std::string& format, Args... args);

    template<typename... Args>
    void error(const std::string& format, Args... args);

private:
    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel level;
    std::ofstream logFile;

    void log(LogLevel level, const std::string& message);

    template<typename T>
    void formatHelper(std::ostringstream& oss, T value);

    template<typename T, typename... Args>
    void formatHelper(std::ostringstream& oss, T value, Args... args);
};

} 
} 

template<typename... Args>
void j2me::core::Logger::debug(const std::string& format, Args... args) {
    if (level > LogLevel::DEBUG) return;
    
    std::ostringstream oss;
    formatHelper(oss, args...);
    debug(format + oss.str());
}

template<typename... Args>
void j2me::core::Logger::info(const std::string& format, Args... args) {
    if (level > LogLevel::INFO) return;
    
    std::ostringstream oss;
    formatHelper(oss, args...);
    info(format + oss.str());
}

template<typename... Args>
void j2me::core::Logger::error(const std::string& format, Args... args) {
    if (level > LogLevel::ERROR) return;
    
    std::ostringstream oss;
    formatHelper(oss, args...);
    error(format + oss.str());
}

template<typename T>
void j2me::core::Logger::formatHelper(std::ostringstream& oss, T value) {
    oss << value;
}

template<typename T, typename... Args>
void j2me::core::Logger::formatHelper(std::ostringstream& oss, T value, Args... args) {
    oss << value;
    formatHelper(oss, args...);
}

#define LOG_DEBUG(msg) j2me::core::Logger::getInstance().debug(msg)
#define LOG_INFO(msg) j2me::core::Logger::getInstance().info(msg)
#define LOG_ERROR(msg) j2me::core::Logger::getInstance().error(msg)
