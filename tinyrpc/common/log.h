#pragma once

#include <string>
#include <queue>
#include <memory>

#include "tinyrpc/common/mutex.h"
#include "tinyrpc/common/config.h"

namespace tinyrpc {

template<typename... Args>
std::string formatString(const char* str, Args... args) {

    int size = snprintf(nullptr, 0, str, args...);

    std::unique_ptr<char[]> buf(new char[size + 1]);
    snprintf(buf.get(), size + 1, str, args...);

    return std::string(buf.get(), buf.get() + size);
}

// todo 重载 << 操作符 LOG_ERROR << "Unknown event type";


#define DEBUGLOG(str, ...) \
if (tinyrpc::Logger::GetGlobalLogger()->getLogLevel() <= tinyrpc::LogLevel::Debug) \
{ \
    tinyrpc::Logger::GetGlobalLogger()->pushLog((new tinyrpc::LogEvent(tinyrpc::LogLevel::Debug))->toString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + tinyrpc::formatString(str, ##__VA_ARGS__) + "\n");\
    tinyrpc::Logger::GetGlobalLogger()->log();                                                                                \
} \

#define INFOLOG(str, ...) \
if (tinyrpc::Logger::GetGlobalLogger()->getLogLevel() <= tinyrpc::Info) \
{ \
    tinyrpc::Logger::GetGlobalLogger()->pushLog((new tinyrpc::LogEvent(tinyrpc::LogLevel::Info))->toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + tinyrpc::formatString(str, ##__VA_ARGS__) + "\n");\
    tinyrpc::Logger::GetGlobalLogger()->log();                                                                      \
} \

#define ERRORLOG(str, ...) \
if (tinyrpc::Logger::GetGlobalLogger()->getLogLevel() <= tinyrpc::Error) \
{ \
    tinyrpc::Logger::GetGlobalLogger()->pushLog((new tinyrpc::LogEvent(tinyrpc::LogLevel::Error))->toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + tinyrpc::formatString(str, ##__VA_ARGS__) + "\n");\
    tinyrpc::Logger::GetGlobalLogger()->log();                                                                                 \
} \

enum LogLevel {
    Unknow = 0,
    Debug,
    Info,
    Error
};

std::string LogLevelToString(LogLevel level);
LogLevel StringToLogLevel(const std::string& log_level);

class Logger {
public:
    using s_ptr = std::shared_ptr<Logger>;

    Logger(LogLevel level) : set_level_(level) {}

    void pushLog(const std::string& log);

    void log();

    LogLevel getLogLevel() const { return set_level_; }

public:
    static Logger* GetGlobalLogger();

    static void InitGlobalLogger();

private:
    LogLevel set_level_;
    std::queue<std::string> log_queue_;

    Mutex mutex_;
};

class LogEvent {
public:
    LogEvent(LogLevel level) : level_(level) {}

    std::string getFileName() const { return file_name_; }

    LogLevel getLogLevel() const { return level_; }

    std::string toString();

private:
    std::string file_name_;     // file name
    int32_t file_line_;         // file line
    int32_t thread_id_;        // thread id
    int32_t pid_;           // process id

    LogLevel level_;        // log level
};

}   // namespace tinyrpc




