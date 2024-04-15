#include "Logger.h"
#include "TimeStamp.h"

#include <iostream>

Logger* Logger::instance_ = new Logger;

// 获取日志唯一的实例对象
Logger* Logger::instance(){
    return instance_;
}

// 设置日志级别
void Logger::setLogLevel(int level){
    logLevel_ = level;
}

// 写日志  [级别信息] time : msg
void Logger::log(enum LogLevel level ,std::string msg){
    
    if(logLevel_>level) return;

    switch (level){
    case INFO:
        std::cout << "[INFO] ";
        break;
    case ERROR:
        std::cout << "[ERROR] ";
        break;
    case FATAL:
        std::cout << "[FATAL] ";
        break;
    case DEBUG:
        std::cout << "[DEBUG] ";
        break;
    default:
        break;
    }

    // 打印时间和msg
    std::cout << TimeStamp::now().toString() << " : " << msg << std::endl;
}