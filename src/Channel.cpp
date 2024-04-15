#include"Channel.h"

//更新fd的监听事件
void Channel::update(){
    loop_->update(this); //让eventloop类更新此channel的监听事件 
} 
 
 //处理事件
void Channel::handleEvent(TimeStamp receiveTime){
    if(revents_ & (EPOLLIN | EPOLLPRI)){
        // 读回调
        if(readCallBack_) readCallBack_(receiveTime);
    }else if(revents_ & (EPOLLOUT)){
        // 写回调
        if(writeCallBack_) writeCallBack_();
    }else if(revents_ & (EPOLLRDHUP)){
        disableAll(); //取消epoll中的fd
        //连接关闭回调
        if(closeCallBack_) closeCallBack_();
    }else{
        disableAll(); //取消epoll中的fd
        // 错误回调
        if(errorCallBack_) errorCallBack_();
    }
}

Channel::Channel(int fd, EventLoop* loop)
    :fd_(fd),
    loop_(loop){

}

Channel::~Channel(){
    disableAll();
}

 //返回fd
int Channel::getFd(){
    return fd_;
}

 //返回监听的事件
int Channel::getEvents(){
    return events_;
}

 //设置发生的事件
void Channel::setREvents(int event){
    revents_ = event;
}

//返回是否注册到事件循环
bool Channel::getIsRegist(){
    return isRegist_;
} 

//设置是否注册到事件循环
void Channel::setIsRegist(bool on){
    isRegist_ = on;
} 

// 设置fd的事件状态
 //监听读事件
void Channel::enableReading(){
    events_ |= EPOLLIN;
    update();
}

 //停止监听读事件
void Channel::disableReading(){
    events_ ^= EPOLLIN;
    update();
}

 //监听写事件
void Channel::enableWriteing(){
    events_ |= EPOLLOUT;
    update();
}

 //停止监听写事件
void Channel::disableWriteing(){
    events_ ^= EPOLLOUT;
    update();
}

 //取消监听所有事件（将监听下树.重置isRegist）
void Channel::disableAll(){
    events_ = 0;
    revents_ = 0;
    loop_->remove(this); //将此channel从树中删除
}

 //设置读回调
void Channel::setReadCallBack(ReadEventCallBack func){
    readCallBack_ = std::move(func);
}

//设置写回调
void Channel::setWriteCallBack(EventCallBack func){
    writeCallBack_ = std::move(func);
} 

 //设置连接断开回调
void Channel::setCloseCallBack(EventCallBack func){
    closeCallBack_ = std::move(func);
}

 //设置错误回调
void Channel::setErrorCallBack(EventCallBack func){
    errorCallBack_ = std::move(func);
}
