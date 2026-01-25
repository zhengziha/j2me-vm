#include "Logger.hpp"
#include <fstream>
#include <chrono>
#include <iomanip>

namespace j2me {
namespace core {

Logger::Logger() : level(LogLevel::INFO) {
    // 打开日志文件
    logFile.open("j2me-vm.log", std::ios::out | std::ios::trunc);
    if (logFile.is_open()) {
        logFile << "J2ME VM Log File" << std::endl;
        logFile << "=================" << std::endl;
    }
}

Logger::~Logger() {
    // 关闭日志文件
    if (logFile.is_open()) {
        logFile.close();
    }
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

    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream timeStream;
    timeStream << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");

    // 使用printf函数输出日志，这样在Switch平台上就能够看到日志输出了
    printf("%s %s %s\n", timeStream.str().c_str(), levelStr, message.c_str());

    // 将日志输出到文件中
    if (logFile.is_open()) {
        logFile << timeStream.str() << " " << levelStr << " " << message << std::endl;
        logFile.flush();
    }
}

} 
} 
