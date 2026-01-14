#include "Logger.hpp"

namespace j2me {
namespace core {

Logger::Logger() : level(LogLevel::INFO) {
}

Logger::~Logger() {
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLevel(LogLevel level) {
    this->level = level;
}

LogLevel Logger::getLevel() const {
    return level;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < this->level) {
        return;
    }

    const char* levelStr;
    switch (level) {
        case LogLevel::DEBUG:
            levelStr = "[DEBUG]";
            break;
        case LogLevel::INFO:
            levelStr = "[INFO]";
            break;
        case LogLevel::ERROR:
            levelStr = "[ERROR]";
            break;
        default:
            levelStr = "[UNKNOWN]";
            break;
    }

    if (level == LogLevel::ERROR) {
        std::cerr << levelStr << " " << message << std::endl;
    } else {
        std::cout << levelStr << " " << message << std::endl;
    }
}

} 
} 
