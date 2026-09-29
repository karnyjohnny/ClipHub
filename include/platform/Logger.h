#pragma once

#include <string>
#include <fstream>
#include <mutex>

namespace cliphub {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static Logger& instance();

    void init(const std::string& logFilePath = "ClipHub.log", LogLevel minLevel = LogLevel::Info);
    void log(LogLevel level, const std::string& message);

    void info(const std::string& msg) { log(LogLevel::Info, msg); }
    void warn(const std::string& msg) { log(LogLevel::Warning, msg); }
    void error(const std::string& msg) { log(LogLevel::Error, msg); }
    void debug(const std::string& msg) { log(LogLevel::Debug, msg); }

    void setLevel(LogLevel level) { m_minLevel = level; }

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    std::ofstream m_file;
    LogLevel m_minLevel = LogLevel::Info;
    std::mutex m_mutex;
};

#define LOG_INFO(msg)  cliphub::Logger::instance().info(msg)
#define LOG_WARN(msg)  cliphub::Logger::instance().warn(msg)
#define LOG_ERROR(msg) cliphub::Logger::instance().error(msg)
#define LOG_DEBUG(msg) cliphub::Logger::instance().debug(msg)

} // namespace cliphub
