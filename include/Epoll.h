#pragma once

#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<strings.h>
#include<sys/epoll.h>
#include<vector>
#include<unistd.h>
#include"Channel.h"
#include"TimeStamp.h"
#include"Logger.h"

class Channel;

class Epoll{
private:
    static const int MaxEventSize = 256; //epoll_wait()返回事件数组大小
    int epollfd_ = -1; //epollfd在构造函数中创建
    epoll_event events_[MaxEventSize]; //存放epoll_wait()返回的事件数组
public:
    Epoll();
    ~Epoll();

    std::vector<Channel*> loop(int timeOut=-1); //运行epoll_wait()

    void updateChannel(Channel* ch); //将ch事件注册或更新
    void removeChannel(Channel* ch); //将ch事件注销（下树）
};