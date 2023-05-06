#include <time.h>
#include <stdio.h>
#include <sstream>
#include "tinyRPC/common/log.h"
#include "tinyRPC/common/util.h"

#define DEBUGLOG(str, ...) \
    std::string msg = (new tinyRPC::LogEvent(tinyRPC::LogLevel::Debug)->toString()) + tinyRPC::formatString(str, ##__VA_ARGS__); \
    tinyRPC::g_logger->pushLog(msg); \
    tinyRPC::g_logger->log();

namespace tinyRPC {

static Logger* g_logger = nullptr;

Logger* Logger::GetGlobalLogger()
{
    if (g_logger == nullptr)
    {
        g_logger = new Logger();
    }
    return g_logger;
}

std::string LogLevelToString(LogLevel level)
{
    switch (level)
    {
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

std::string LogEvent::toString()
{
    struct timeval tv;
    
    struct tm* time;
    localtime_r(&tv.tv_sec, time);

    char buf[128];
    strftime(buf, 128, "%Y-%m-%d %H:%M:%S", time);
    std::string time_str(time_str);
    int ms = tv.tv_usec / 1000;
    time_str = time_str + "." + std::to_string(ms);

    m_pid = getPid();
    m_thread_id = getThreadId();

    std::stringstream ss;

    ss << "[" << LogLevelToString(m_level) << "]\t"
    << "[" << time_str << "]\t"
    << "[" << std::string(__FILE__) << ":" << __LINE__ << "]\t";

    return ss.str();
}

void Logger::pushLog(const std::string& msg)
{
    m_buffer.push(msg);
}

void Logger::log()
{
    while (!m_buffer.empty())
    {
        std::string msg = m_buffer.front();
        m_buffer.pop();
        printf("%s\n", msg.c_str());
    }
}

};