#include"TcpConnection.h"

TcpConnection::TcpConnection(EventLoop* loop, std::unique_ptr<Socket> clientsock)
    :loop_(loop),
    clientSock_(std::move(clientsock)),
    clientChannel_(new Channel(clientSock_->fd(), loop_)),
    disconnection_(false),
    lastTime_(TimeStamp::now()){
        // 绑定回调
    clientChannel_->setReadCallBack(std::bind(&TcpConnection::handleMessage, this, std::placeholders::_1));
    clientChannel_->setWriteCallBack(std::bind(&TcpConnection::handleWrite, this));
    clientChannel_->setCloseCallBack(std::bind(&TcpConnection::handleClose, this));
    clientChannel_->setErrorCallBack(std::bind(&TcpConnection::handleError, this));
    clientChannel_->enableReading();
}

TcpConnection::~TcpConnection(){
}

 //返回连接socket fd
int TcpConnection::getFd() const{
    return clientChannel_->getFd();
}

 //返回连接ip
std::string TcpConnection::getIp() const{
    return clientSock_->getIp();
}

 //返回连接port
uint16_t TcpConnection::getPort() const{
    return clientSock_->getPort();
}

 //TCP连接关闭的回调函数
void TcpConnection::handleClose(){
    disconnection_ = true;
    clientChannel_->disableAll();
    if(closeCallBack_) closeCallBack_(shared_from_this());
    // 删除事件循环中此对象
    loop_->removeConnection(shared_from_this());
}

 //TCP连接错误的回调函数
void TcpConnection::handleError(){
    disconnection_ = true;
    clientChannel_->disableAll();
    if(errorCallBack_) errorCallBack_(shared_from_this());
    loop_->removeConnection(shared_from_this());
}

//TCP写回调函数
void TcpConnection::handleWrite(){
    if(disconnection_) return;
    int savedErrorno = 0;
    // 数据发送
    int n = outputBuffer_.writeFd(getFd(), &savedErrorno);
    if(n>0){
        // 发送成功
        if(outputBuffer_.readableBytes()<=0){
            // 数据全部发送完成
            // 取消监听
            clientChannel_->disableWriteing();
            if(sendCompleteCallBack_) sendCompleteCallBack_(shared_from_this());
        }
    }else{
        // 发送失败
        LOG_ERROR("outputBuffer_.writeFd error:%d\n", savedErrorno);
        // 取消写事件监听
        // clientChannel_->disableWriteing();
    }
}

 //处理对端数据
void TcpConnection::handleMessage(TimeStamp tm){
    // 更新时间戳
    lastTime_ = tm;
    if(disconnection_) return;
    // 从buffer中获取数据
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(clientChannel_->getFd(), &savedErrno); //将数据读入缓冲区
    if(n<0){
        handleError();
        return;
    }else if(n == 0){
        // 连接断开
        handleClose();
        return;
    }
    std::string packData = bufferGetPackCallBack_(inputBuffer_);
    if(packData.size()==0||packData.length()==0){
        // LOG_INFO("client[%s:%d] not read a complete pack.", getIp().c_str(), getPort());
        return ;
    }
    // 回调
    if(onmessageCallBack_) 
        onmessageCallBack_(shared_from_this(), tm, std::move(packData));
}

 //设置TCP连接关闭的回调函数
void TcpConnection::setCloseCallBack(std::function<void(WptrConnection)> func){
    closeCallBack_ = std::move(func);
}

 //设置TCP连接错误的回调函数
void TcpConnection::setErrorCallBack(std::function<void(WptrConnection)> func){
    errorCallBack_ = std::move(func);
}

 //设置TCP连接数据读取的回调函数
void TcpConnection::setOnmessageCallBack(std::function<void(WptrConnection, TimeStamp, std::string)> func){
    onmessageCallBack_ = std::move(func);
}

 //设置发送数据完成回调函数
void TcpConnection::setSendCompleteCallBack(std::function<void(WptrConnection)> func){
    sendCompleteCallBack_ = std::move(func);
}

 //设置接收缓冲区获取单个数据包方法 （分包方法）
void TcpConnection::setBufferGetPackCallBack(std::function<std::string (Buffer&)> func){
    bufferGetPackCallBack_ = std::move(func);
}

 //设置数据写入缓冲区方法 （组包方法）
void TcpConnection::setBufferAppendPackCallBack(std::function<void (Buffer&, std::string)> func){
    bufferAppendPackCallBack_ = std::move(func);
}

//发送数据给客户端，IO线程
void TcpConnection::sendInIOThread(std::string data){
    //组包
    bufferAppendPackCallBack_(outputBuffer_, data);
    // 开始监听可写事件
    clientChannel_->enableWriteing();
} 

//发送数据给客户端，word线程
void TcpConnection::sendInWorkThread(std::string data){
    // 将work提交给IO线程
    loop_->addTaskToIOThread(std::bind(&TcpConnection::sendInIOThread, this, data));
}


//发送数据给客户端，不管在任何线程中都调用此函数发送数据
void TcpConnection::send(std::string& data){ //这个函数可能在io线程或工作线程中执行
    if(loop_->curThreadIsIOThread()){
        // 在IO线程中
        sendInIOThread(data);
    }else{
        // 在work线程中
        sendInWorkThread(data);
    }
}

 //判断TCP连接是否超时（空闲太久）
bool TcpConnection::timeout(TimeStamp now, int connectionMaxSec){
    return now.sub(lastTime_).toSec()>connectionMaxSec;
}