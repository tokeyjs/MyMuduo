#pragma once

#include<functional>
#include"Socket.h"
#include"InetAddress.h"
#include"Channel.h"
#include"EventLoop.h"
#include<memory>

class TcpAcceptor{
private:
    EventLoop* loop_; // Acceptor类对应的事件循环在构造函数中传入
    Socket serverSock_; //服务器监听的sokcet，在构造函数中创建
    Channel acceptChannel_; //Acceptor对应地channnel在构造函数中创建
    std::function<void(std::unique_ptr<Socket>)> newConnectionCallBack_; //新连接回调函数
public:
    TcpAcceptor(EventLoop* loop, const std::string &ip, const uint16_t port);
    ~TcpAcceptor();

    void handleNewConnection(); //处理新客户端连接请求
    void setNewConnectionCallBack(std::function<void(std::unique_ptr<Socket>)> func); //设置新连接回调函数
};