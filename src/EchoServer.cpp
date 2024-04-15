#include"EchoServer.h"


//subthreadnum从事件循环数量， workthreadnum工作线程数量
EchoServer::EchoServer(const char* ip, uint16_t port, int subthreadnum, int workthreadnum)
    :tcpServer_(ip, port, subthreadnum, workthreadnum){
        // 绑定处理函数
        tcpServer_.setOnMessaggeCallBack(std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
        tcpServer_.setNewConnectionCallBack(std::bind(&EchoServer::HandleNewConnection, this, std::placeholders::_1));
        tcpServer_.setCloseConnectionCallBack(std::bind(&EchoServer::HandleClose, this, std::placeholders::_1)); 
        tcpServer_.setErrorConnectionCallBack(std::bind(&EchoServer::HandleError, this, std::placeholders::_1)); 
        tcpServer_.setEventLoopTimeOutCallBack(std::bind(&EchoServer::HandleTimeout, this, std::placeholders::_1)); 
} 

EchoServer::~EchoServer(){

}

 //服务启动
void EchoServer::Start(){
    tcpServer_.run();
}

 //停止服务
void EchoServer::Stop(){
    tcpServer_.stop();
}

 // 新连接处理函数 
void EchoServer::HandleNewConnection(TcpConnection::WptrConnection conn){
    if(auto sconn = conn.lock()){

    LOG_INFO("[+] new client[%s:%d] connection.\n",sconn->getIp().c_str(), sconn->getPort());
    }
}

// 客户端连接关闭，回调函数
void EchoServer::HandleClose(TcpConnection::WptrConnection conn){
    if(auto sconn = conn.lock()){

    LOG_INFO("[-] client[%s:%d] disconnection.\n",sconn->getIp().c_str(), sconn->getPort());
    }
} 

// 客户端连接错误，回调函数
void EchoServer::HandleError(TcpConnection::WptrConnection conn){
    if(auto sconn = conn.lock()){
    LOG_INFO("[-] client[%s:%d] error disconnection.\n",sconn->getIp().c_str(), sconn->getPort());

    }
} 

 //处理客户端请求数据，在Conection类中回调此函数
void EchoServer::HandleMessage(TcpConnection::WptrConnection conn, TimeStamp tm,std::string message){
    if(auto sconn = conn.lock()){
            // LOG_INFO("client[%s:%d][%s]: %s\n",sconn->getIp().c_str(), sconn->getPort(),tm.toString().c_str(), message.c_str());
            sconn->send(message);
    }

}

//数据发送完成后调用此函数
void EchoServer::HandleSendComplete(TcpConnection::WptrConnection conn){

} 

 //epoll_wait()超时，在eventloop中回调此函数
void EchoServer::HandleTimeout(EventLoop* loop){
    LOG_INFO("eventloop timeout.\n");
}
