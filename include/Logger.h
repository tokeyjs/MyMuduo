#pragma once

#include <string>

// log开关
// #define NOLOG

// LOG_INFO("%s %d", arg1, arg2)
#ifndef NOLOG
#define LOG_INFO(logmsgFormat, ...) \
    do \
    { \
        Logger *logger = Logger::instance(); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger->log(INFO, buf); \
    } while(0) 
#else
#define LOG_INFO(logmsgFormat, ...) 
#endif

#ifndef NOLOG
#define LOG_ERROR(logmsgFormat, ...) \
    do \
    { \
        Logger *logger = Logger::instance(); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger->log(ERROR, buf); \
    } while(0) 
#else
#define LOG_ERROR(logmsgFormat, ...) 
#endif

#ifndef NOLOG
#define LOG_FATAL(logmsgFormat, ...) \
    do \
    { \
        Logger *logger = Logger::instance(); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger->log(FATAL, buf); \
        exit(-1); \
    } while(0) 
#else
#define LOG_FATAL(logmsgFormat, ...) 
#endif

#ifndef NOLOG
#define LOG_DEBUG(logmsgFormat, ...) \
    do \
    { \
        Logger *logger = Logger::instance(); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger->log(DEBUG, buf); \
    } while(0) 
#else
#define LOG_DEBUG(logmsgFormat, ...) 
#endif


// 定义日志的级别  INFO  ERROR  FATAL  DEBUG 
enum LogLevel
{   
    DEBUG, // 调试信息
    INFO,  // 普通信息
    ERROR, // 错误信息
    FATAL, // core信息
};

// 输出一个日志类
class Logger
{
public:
    Logger(enum LogLevel level=DEBUG):logLevel_(level){}
    // 获取日志唯一的实例对象
    static Logger* instance();
    // 设置日志级别
    void setLogLevel(int level);
    // 写日志
    void log(enum LogLevel level, std::string msg);

    static Logger* instance_; // 单例

private:
    int logLevel_;
};