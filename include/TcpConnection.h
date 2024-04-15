#pragma once

#include<functional>
#include"Socket.h"
#include"Channel.h"
#include"InetAddress.h"
#include"EventLoop.h"
#include"Buffer.h"
#include"TimeStamp.h"
#include<memory>
#include<atomic>

class EventLoop;
class Channel;


class TcpConnection: public std::enable_shared_from_this<TcpConnection>{
public:
        // connection智能指针 
    using WptrConnection = std::weak_ptr<TcpConnection>;
private:

    EventLoop* loop_; //对应的事件循环
    std::unique_ptr<Socket> clientSock_;  //对应的socket
    std::unique_ptr<Channel> clientChannel_; // connection 对应的channel
    Buffer inputBuffer_; // 接收缓冲区
    Buffer outputBuffer_; // 发送缓冲区
    std::atomic_bool disconnection_; //连接是否已经断开
    TimeStamp lastTime_; //时间戳，创建connection对象时为当前时间，每接受一个报文，把时间戳更新为当前时间
    
    std::function<void(WptrConnection)> closeCallBack_; //客户端关闭回调函数
    std::function<void(WptrConnection)> errorCallBack_; //客户端错误回调函数
    std::function<void(WptrConnection)> sendCompleteCallBack_; //缓冲区数据发送完成回调函数
    std::function<void(WptrConnection, TimeStamp, std::string)> onmessageCallBack_; //客户端数据处理回调函数

    // 组包回调函数
    std::function<void (Buffer&, std::string)> bufferAppendPackCallBack_; 
    // 解包回调函数
    std::function<std::string (Buffer&)> bufferGetPackCallBack_;

    void sendInIOThread(std::string data); //发送数据给客户端，IO线程
    void sendInWorkThread(std::string data); //发送数据给客户端，word线程

public:
    TcpConnection(EventLoop* loop, std::unique_ptr<Socket> clientsock);
    ~TcpConnection();

    int getFd() const; //返回连接socket fd
    std::string getIp() const; //返回连接ip
    uint16_t getPort() const; //返回连接port
    
    void handleClose(); //TCP连接关闭的回调函数
    void handleError(); //TCP连接错误的回调函数
    void handleWrite(); //TCP写回调函数
    void handleMessage(TimeStamp tm); //处理对端数据

    void setCloseCallBack(std::function<void(WptrConnection)> func); //设置TCP连接关闭的回调函数
    void setErrorCallBack(std::function<void(WptrConnection)> func); //设置TCP连接错误的回调函数
    void setOnmessageCallBack(std::function<void(WptrConnection, TimeStamp, std::string)> func); //设置TCP连接数据读取的回调函数
    void setSendCompleteCallBack(std::function<void(WptrConnection)> func); //设置发送数据完成回调函数

    void setBufferGetPackCallBack(std::function<std::string (Buffer&)> func); //设置接收缓冲区获取单个数据包方法 （分包方法）
    void setBufferAppendPackCallBack(std::function<void (Buffer&, std::string)> func); //设置数据写入缓冲区方法 （组包方法）

    void send(std::string& data); //发送数据给客户端，不管在任何线程中都调用此函数发送数据
    bool timeout(TimeStamp now, int connectionMaxSec=12); //判断TCP连接是否超时（空闲太久）
};