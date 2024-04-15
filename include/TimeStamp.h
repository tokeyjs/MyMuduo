#pragma once

#include <string>
#include<sys/time.h>

// 时间戳类

class TimeStamp
{
public:
    TimeStamp();
    explicit TimeStamp(int64_t microSecondsSinceEpoch);  
    static TimeStamp now();
    std::string toString(bool showMicroseconds=true) const; //默认带微秒数
    TimeStamp sub(TimeStamp& timeStamp); //时间相减
    int toSec(); //转成秒级时间戳
    int toMilSec(); //转成毫秒级时间戳
private:
    int64_t microSecondsSinceEpoch_; //微秒为单位
};