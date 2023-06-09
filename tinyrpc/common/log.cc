#include <sys/time.h>
#include <sstream>
#include <cstdio>

#include "tinyrpc/common/log.h"
#include "tinyrpc/common/util.h"
#include "tinyrpc/common/config.h"

namespace tinyrpc {

// static Logger::s_ptr g_logger = nullptr;
static Logger* g_logger = nullptr;


Logger* Logger::GetGlobalLogger() {
    // return g_logger.get();
    return g_logger;
}

void Logger::InitGlobalLogger() {
    LogLevel level = StringToLogLevel(Config::GetGlobalConfig()->log_level_);
    printf("Init log level [%s]\n", LogLevelToString(level).c_str());
    // g_logger = std::make_shared<Logger>(level);
    g_logger = new Logger(level);
}


std::string LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Error:
            return "ERROR";
        default:
            return "UNKNOW";
    }
}

LogLevel StringToLogLevel(const std::string& log_level) {
    if (log_level == "DEBUG") {
        return LogLevel::Debug;
    } else if (log_level == "INFO") {
        return LogLevel::Info;
    } else if (log_level == "ERROR") {
        return LogLevel::Error;
    } else {
        return LogLevel::Unknow;
    }
}

std::string LogEvent::toString() {
    struct timeval now_time;

    gettimeofday(&now_time, nullptr);

    struct tm now_time_t;
    localtime_r(&now_time.tv_sec, &now_time_t);

    char buf[128];
    // warn
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &now_time_t);
    std::string time_str(buf);
    int ms = now_time.tv_usec / 1000;
    time_str += "." + std::to_string(ms);

    pid_ = getpid();
    thread_id_ = getThreadId();

    std::stringstream ss;

    ss << "[" << LogLevelToString(level_) << "]\t"
       << "[" << time_str << "]\t"
       << "[" << pid_ << ":" << thread_id_ << "]\t";
    
    return ss.str();
}

void Logger::pushLog(const std::string& log) {
    ScopeMutex<Mutex> lock(mutex_);
    log_queue_.push(log);
    lock.unlock();
}

void Logger::log() {

    ScopeMutex<Mutex> lock(mutex_);    
    std::queue<std::string> tmp = std::move(log_queue_);
    // log_queue_.swap(tmp);

    lock.unlock();

    while (!tmp.empty()) {
        std::string log = tmp.front();
        tmp.pop();
        printf(log.c_str());
    }
}

}   // namespace tinyrpc