#pragma once
#include"EventLoop.h"
#include"TimeStamp.h"
#include<functional>
#include<memory>
#include <sys/epoll.h>

class EventLoop;

class Channel{
    using EventCallBack = std::function<void()>;
    using ReadEventCallBack = std::function<void(TimeStamp)>;
private:
    const int fd_ = -1; //当前channel对应的fd对象
    EventLoop *loop_ = nullptr; //当前channel所属事件循环
    int events_ = 0; //注册的fd监听事件
    int revents_ = 0; //fd发生的事件(epoll返回)
    bool isRegist_ = false; //事件是否已经注册

    ReadEventCallBack readCallBack_; //读回调
    EventCallBack writeCallBack_; //写回调
    EventCallBack closeCallBack_; //连接断开回调
    EventCallBack errorCallBack_; //错误回调

    void update(); //更新fd的监听事件
public:
    Channel(int fd, EventLoop* loop);
    ~Channel();

    void handleEvent(TimeStamp receiveTime); //处理事件
    
    int getFd(); //返回fd
    int getEvents(); //返回监听的事件
    void setREvents(int event); //设置发生的事件
    bool getIsRegist(); //返回是否注册到事件循环
    void setIsRegist(bool on); //设置是否注册到事件循环

    // 设置fd的事件状态
    void enableReading(); //监听读事件
    void disableReading(); //停止监听读事件
    void enableWriteing(); //监听写事件
    void disableWriteing(); //停止监听写事件
    void disableAll(); //取消监听所有事件（将监听下树.重置isRegist）

    void setReadCallBack(ReadEventCallBack func); //设置读回调
    void setWriteCallBack(EventCallBack func); //设置写回调
    void setCloseCallBack(EventCallBack func); //设置连接断开回调
    void setErrorCallBack(EventCallBack func); //设置错误回调

};