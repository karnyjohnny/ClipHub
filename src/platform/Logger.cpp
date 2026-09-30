#include "platform/Logger.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace cliphub {

Logger& Logger::instance() {
    static Logger s_instance;
    return s_instance;
}

Logger::~Logger() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file.is_open()) {
        m_file.close();
    }
}

void Logger::init(const std::string& logFilePath, LogLevel minLevel) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel = minLevel;
    if (m_file.is_open()) {
        m_file.close();
    }
    m_file.open(logFilePath, std::ios::out | std::ios::app);
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < m_minLevel) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm bt{};
#if defined(_WIN32)
    localtime_s(&bt, &in_time_t);
#else
    localtime_r(&in_time_t, &bt);
#endif

    const char* levelStr = "INFO";
    switch (level) {
        case LogLevel::Debug:   levelStr = "DEBUG"; break;
        case LogLevel::Info:    levelStr = "INFO "; break;
        case LogLevel::Warning: levelStr = "WARN "; break;
        case LogLevel::Error:   levelStr = "ERROR"; break;
    }

    std::ostringstream ss;
    ss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S") << '.' 
       << std::setfill('0') << std::setw(3) << ms.count()
       << " [" << levelStr << "] " << message << "\n";

    std::string out = ss.str();
    if (m_file.is_open()) {
        m_file << out;
        m_file.flush();
    }
}

} // namespace cliphub
