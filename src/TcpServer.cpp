#include"TcpServer.h"


TcpServer::TcpServer(std::string ip, uint16_t port, size_t subReactorNum, size_t workThreadNum)
    :mainLoop_(new EventLoop(true)),
    subReactorNum_(subReactorNum),
    workThreadNum_(workThreadNum),
    subLoopPool_(subReactorNum_),
    workThreadPool_(workThreadNum_),
    acceptor_(mainLoop_.get(), ip, port){
        
    mainLoop_->setEpollTimeOutCallBack(std::bind(&TcpServer::handleEventLoopTimeOut, this, std::placeholders::_1));
    acceptor_.setNewConnectionCallBack(std::bind(&TcpServer::handleNewConnection, this, std::placeholders::_1));

    // 设置默认buffer分包组包函数
    setBufferGetPackCallBack(std::bind(TcpServer::CommonBufferGetPackNone, std::placeholders::_1));
    setBufferAppendPackCallBack(std::bind(TcpServer::CommonBufferAppendPackNone, std::placeholders::_1, std::placeholders::_2));

    for(int i=0;i<subReactorNum_;i++){
        subLoops_.emplace_back(new EventLoop(false));
        subLoops_[i]->setEpollTimeOutCallBack(std::bind(&TcpServer::handleEventLoopTimeOut, this, std::placeholders::_1));
        subLoops_[i]->setTimerCallBack(std::bind(&TcpServer::handleRemoveConn, this, std::placeholders::_1));
        subLoopPool_.addTask(std::bind(&EventLoop::start, subLoops_[i].get()));
    }
}

TcpServer::~TcpServer(){
    stop();
}

 // 运行事件循环
void TcpServer::run(){
    mainLoop_->start();
}

 //停止事件循环
void TcpServer::stop(){
    mainLoop_->stop();
    for(auto& lp:subLoops_)
        lp->stop();
    subLoopPool_.stop();
    workThreadPool_.stop();
    subLoops_.clear();
}

// 清除空闲连接
void TcpServer::handleRemoveConn(int fd){
    std::lock_guard<std::mutex> loc(connMapMutex_);
    conns_.erase(fd);
} 

//设置回调
void TcpServer::setNewConnectionCallBack(ConnectionCallBack func){
    newConnectionCallBack_ = std::move(func);
}

void TcpServer::setCloseConnectionCallBack(ConnectionCallBack func){
    closeConnectionCallBack_ = std::move(func);
}

void TcpServer::setErrorConnectionCallBack(ConnectionCallBack func){
    errorConnectionCallBack_ = std::move(func);
}

void TcpServer::setSendCompleteCallBack_(ConnectionCallBack func){
    sendCompleteCallBack_ = std::move(func);
}

void TcpServer::setOnMessaggeCallBack(OnMessageCallBack func){
    onMessaggeCallBack_ = func;
}

void TcpServer::setEventLoopTimeOutCallBack(TimeOutCallBack func){
    eventLoopTimeOutCallBack_ = std::move(func);
}

void TcpServer::setBufferGetPackCallBack(GetPackCallBack func){
    bufferGetPackCallBack_ = std::move(func);
}

void TcpServer::setBufferAppendPackCallBack(AppendPackCallBack func){
    bufferAppendPackCallBack_ = std::move(func);
}


void TcpServer::handleNewConnection(std::unique_ptr<Socket> clientSock){
    // 将连接分配给从事件循环
    std::shared_ptr<TcpConnection> conn(new TcpConnection(subLoops_[clientSock->fd()%subReactorNum_].get(),
     std::move(clientSock)));
    // 设置客户端断开回调函数
    conn->setCloseCallBack(std::bind(&TcpServer::handleCloseConnection, this, std::placeholders::_1));
    // 设置客户端错误回调函数
    conn->setErrorCallBack(std::bind(&TcpServer::handleErrorConnection, this, std::placeholders::_1));
    // 设置客户端处理数据的回调函数
    conn->setOnmessageCallBack(std::bind(&TcpServer::handleOnMessagge, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // 设置客户端数据发送完成回调函数
    conn->setSendCompleteCallBack(std::bind(&TcpServer::handleSendComplete, this, std::placeholders::_1));
    // 设置缓冲区 组包函数
    conn->setBufferAppendPackCallBack(bufferAppendPackCallBack_);
    // 设置缓冲区 解包函数
    conn->setBufferGetPackCallBack(bufferGetPackCallBack_);

    // 将客户端连接放到map中去
    {
        std::lock_guard<std::mutex> loc(connMapMutex_);
        conns_[conn->getFd()] = conn;
    }
    subLoops_[conn->getFd()%subReactorNum_]->addConnection(conn);

    if(newConnectionCallBack_) 
        newConnectionCallBack_(conn); //调用回调
}

void TcpServer::handleCloseConnection(TcpConnection::WptrConnection conn){
    if(closeConnectionCallBack_) 
        closeConnectionCallBack_(conn);
    std::lock_guard<std::mutex> loc(connMapMutex_);
    if(auto sconn = conn.lock()){
            conns_.erase(sconn->getFd());
    }
    
}

void TcpServer::handleErrorConnection(TcpConnection::WptrConnection conn){
    if(errorConnectionCallBack_) 
        errorConnectionCallBack_(conn);
    std::lock_guard<std::mutex> loc(connMapMutex_);
    if(auto sconn = conn.lock()){
            conns_.erase(sconn->getFd());
    }


}

void TcpServer::handleSendComplete(TcpConnection::WptrConnection conn){

    if(sendCompleteCallBack_) 
        sendCompleteCallBack_(conn);
}

void TcpServer::handleOnMessagge(TcpConnection::WptrConnection conn, TimeStamp tieStamp, std::string message){
    if(workThreadNum_>0){
        // 含有工作线程
        if(onMessaggeCallBack_)
            workThreadPool_.addTask(std::bind(onMessaggeCallBack_, conn, tieStamp, message));
    }else{
        // 没有工作线程，直接在IO线程中处理
        if(onMessaggeCallBack_)
            onMessaggeCallBack_(conn, tieStamp, message);
    }
}

void TcpServer::handleEventLoopTimeOut(EventLoop* eventLoop){
    if(eventLoopTimeOutCallBack_) 
        eventLoopTimeOutCallBack_(eventLoop);
}



//-------分包、组包： 无（default）
std::string TcpServer::CommonBufferGetPackNone(Buffer& buf){
    return buf.retrieveAllAsString();
}

void TcpServer::CommonBufferAppendPackNone(Buffer& buf, std::string data){
    buf.append(data.data(), data.size());
}

//------分包、组包： 指定分隔符-----
std::string TcpServer::CommonBufferGetPackSplitStr(Buffer& buf, std::string splitStr){
    return "";
}

void TcpServer::CommonBufferAppendPackSplitStr(Buffer& buf, std::string data, std::string splitStr){

}

//------分包、组包： 头部长度-----
std::string TcpServer::CommonBufferGetPackWithHead(Buffer& buf){
    return "";
}

void TcpServer::CommonBufferAppendPackWithHead(Buffer& buf, std::string data){

}