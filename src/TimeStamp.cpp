#include "TimeStamp.h"

TimeStamp::TimeStamp():microSecondsSinceEpoch_(0) {}

TimeStamp::TimeStamp(int64_t microSecondsSinceEpoch)
    : microSecondsSinceEpoch_(microSecondsSinceEpoch)
    {}

TimeStamp TimeStamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec + 8 * 60 *60; //加8小时
    return TimeStamp(seconds * 1000 * 1000 + tv.tv_usec);
}

std::string TimeStamp::toString(bool showMicroseconds) const
{
    char buf[64]{0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / (1000*1000));
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);
    if (showMicroseconds){
        int microseconds = static_cast<int>(microSecondsSinceEpoch_ % (1000*1000));
        snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d.%06d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                microseconds);
    }
    else{
        snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

 //时间相减
TimeStamp TimeStamp::sub(TimeStamp& timeStamp){
    return TimeStamp(microSecondsSinceEpoch_-timeStamp.microSecondsSinceEpoch_);
}

 //转成秒级时间戳
int TimeStamp::toSec(){
    return microSecondsSinceEpoch_/(1000*1000);
}

 //转成毫秒级时间戳
int TimeStamp::toMilSec(){
    return microSecondsSinceEpoch_/(1000);
}