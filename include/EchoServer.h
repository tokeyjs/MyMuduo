#pragma once
#include"TcpServer.h"
#include"Logger.h"
#include"Channel.h"
#include"TcpConnection.h"
#include"ThreadPool.h"
#include"TimeStamp.h"


// 回显服务器
class EchoServer{
private:
    TcpServer tcpServer_; // tcp server
public:
    EchoServer(const char* ip, uint16_t port, int subthreadnum=3, int workthreadnum=3); //subthreadnum从事件循环数量， workthreadnum工作线程数量
    ~EchoServer();

    void Start(); //服务启动
    void Stop(); //停止服务
    
    void HandleNewConnection(TcpConnection::WptrConnection conn); // 新连接处理函数 
    void HandleClose(TcpConnection::WptrConnection conn); // 客户端连接关闭，回调函数
    void HandleError(TcpConnection::WptrConnection conn); // 客户端连接错误，回调函数
    void HandleMessage(TcpConnection::WptrConnection conn, TimeStamp tm, std::string message); //处理客户端请求数据，在Conection类中回调此函数
    void HandleSendComplete(TcpConnection::WptrConnection conn); //数据发送完成后调用此函数
    void HandleTimeout(EventLoop* loop); //epoll_wait()超时，在eventloop中回调此函数

};