#include"TcpAcceptor.h"



TcpAcceptor::TcpAcceptor(EventLoop* loop, const std::string &ip, const uint16_t port)
    :loop_(loop),
    serverSock_(createNonBlockingListenFD()),
    acceptChannel_(serverSock_.fd(), loop_){
        serverSock_.setKeepAlive(true);
        serverSock_.setReuseAddr(true);
        serverSock_.setReusePort(true);
        serverSock_.setTcpNoDelay(true);
        serverSock_.bindAddress(InetAddress(ip, port));
        serverSock_.listen();

        // 设置回调函数
        acceptChannel_.setReadCallBack(std::bind(&TcpAcceptor::handleNewConnection, this));
        acceptChannel_.enableReading(); 
}

TcpAcceptor::~TcpAcceptor(){

}

//处理新客户端连接请求
void TcpAcceptor::handleNewConnection(){
    // 接收新客户端
    InetAddress peerAddr;
    int clientFd = serverSock_.accept(peerAddr);
    std::unique_ptr<Socket> clientSock(new Socket(clientFd));
    clientSock->setIp(peerAddr.toIp());
    clientSock->setPort(peerAddr.toPort());
    newConnectionCallBack_(std::move(clientSock));
} 

//设置新连接回调函数
void TcpAcceptor::setNewConnectionCallBack(std::function<void(std::unique_ptr<Socket>)> func){
    newConnectionCallBack_ = std::move(func);
} 