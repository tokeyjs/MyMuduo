#pragma once

#include"EventLoop.h"
#include"Channel.h"
#include"Socket.h"
#include"TcpAcceptor.h"
#include"TcpConnection.h"
#include"TimeStamp.h"
#include"Logger.h"
#include<unordered_map>
#include<vector>
#include"ThreadPool.h"
#include"Buffer.h"
#include<memory>
#include<mutex>

class TcpServer{
private:
    using ConnectionCallBack = std::function<void(TcpConnection::WptrConnection)>;
    using OnMessageCallBack = std::function<void(TcpConnection::WptrConnection, TimeStamp, std::string)>;
    using TimeOutCallBack = std::function<void(EventLoop*)>;
    using GetPackCallBack = std::function<std::string (Buffer&)>;
    using AppendPackCallBack = std::function<void (Buffer&, std::string)>;


    std::unique_ptr<EventLoop> mainLoop_; //主Reactor（主事件循环）
    std::vector<std::unique_ptr<EventLoop>> subLoops_; //从Reactor（从事件循环）  
    int subReactorNum_; //从事件循环（Reactor）数量
    int workThreadNum_; //work线程数量
    ThreadPool subLoopPool_; //从事件循环运行在上面（IO线程）
    ThreadPool workThreadPool_; //work线程（work线程）

    TcpAcceptor acceptor_; // 1-1
    std::unordered_map<int, std::shared_ptr<TcpConnection>> conns_; //保存所有连接 1-n
    std::mutex connMapMutex_; //map锁

    ConnectionCallBack newConnectionCallBack_; //新连接回调函数
    ConnectionCallBack closeConnectionCallBack_; //连接关闭回调函数
    ConnectionCallBack errorConnectionCallBack_; //错误回调函数
    ConnectionCallBack sendCompleteCallBack_; //服务端数据发送完成回调函数
    OnMessageCallBack onMessaggeCallBack_; //客户端数据处理回调函数
    TimeOutCallBack eventLoopTimeOutCallBack_; //服务端超时回调函数

    GetPackCallBack bufferGetPackCallBack_; //分包函数
    AppendPackCallBack bufferAppendPackCallBack_ ; //组包函数

public:
    TcpServer(std::string ip="127.0.0.1", uint16_t port=8080, size_t subReactorNum=2, size_t workThreadNum=3);
    ~TcpServer();

    void run(); // 运行事件循环
    void stop(); //停止事件循环

    //设置回调
    void setNewConnectionCallBack(ConnectionCallBack func);
    void setCloseConnectionCallBack(ConnectionCallBack func);
    void setErrorConnectionCallBack(ConnectionCallBack func);
    void setSendCompleteCallBack_(ConnectionCallBack func);
    void setOnMessaggeCallBack(OnMessageCallBack func);
    void setEventLoopTimeOutCallBack(TimeOutCallBack func);
    void setBufferGetPackCallBack(GetPackCallBack func);
    void setBufferAppendPackCallBack(AppendPackCallBack func);

    void handleNewConnection(std::unique_ptr<Socket> clientSock);
    void handleCloseConnection(TcpConnection::WptrConnection conn);
    void handleErrorConnection(TcpConnection::WptrConnection conn);
    void handleSendComplete(TcpConnection::WptrConnection conn);
    void handleOnMessagge(TcpConnection::WptrConnection conn, TimeStamp tieStamp, std::string message);
    void handleEventLoopTimeOut(EventLoop* eventLoop);
    void handleRemoveConn(int fd); // 清除空闲连接

    //-------分包、组包： 无（default）
    static std::string CommonBufferGetPackNone(Buffer& buf);
    static void CommonBufferAppendPackNone(Buffer& buf, std::string data);

    //------分包、组包： 指定分隔符-----
    static std::string CommonBufferGetPackSplitStr(Buffer& buf, std::string splitStr);
    static void CommonBufferAppendPackSplitStr(Buffer& buf, std::string data, std::string splitStr);

    //------分包、组包： 头部长度-----
    static std::string CommonBufferGetPackWithHead(Buffer& buf);
    static void CommonBufferAppendPackWithHead(Buffer& buf, std::string data);

};