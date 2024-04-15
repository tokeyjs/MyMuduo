#include"Epoll.h"

Epoll::Epoll(){
    epollfd_ = epoll_create(1);
    if(-1 == epollfd_){
        LOG_FATAL("epoll_create() error errno:%d\n", errno);
    }
}

Epoll::~Epoll(){
    ::close(epollfd_);
}

//运行epoll_wait()
std::vector<Channel*> Epoll::loop(int timeOut){
    std::vector<Channel*> channelVec;
    bzero(events_, sizeof(events_));
    int fds = epoll_wait(epollfd_, events_, MaxEventSize, timeOut);
    if(fds<0){
        // 返回失败
        LOG_ERROR("epoll_wait() failed errno:%d", errno);
        return channelVec;
    }else if(fds==0){
        //超时
        return channelVec;
    }
    for(int i=0;i<fds;i++){
        Channel* chan = (Channel*)(events_[i].data.ptr);
        chan->setREvents(events_[i].events);
        channelVec.push_back(chan);
    }
    return channelVec;
} 

 //将ch事件注册或更新
void Epoll::updateChannel(Channel* ch){
    epoll_event ev;
    ev.events = ch->getEvents();
    ev.data.ptr = ch;
    if(!ch->getIsRegist()){
        // 上树
        if(epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->getFd(), &ev)==-1){
            LOG_FATAL("epoll_ctl() EPOLL_CTL_ADD error.\n");
        }
        ch->setIsRegist(true);
    }else{
        // 更新
        if(epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->getFd(), &ev)==-1){
            LOG_FATAL("epoll_ctl() EPOLL_CTL_MOD error.\n");
        }
    }
}

//将ch事件注销（下树）
void Epoll::removeChannel(Channel* ch){
    if(!ch->getIsRegist()) return;
    if(epoll_ctl(epollfd_, EPOLL_CTL_DEL, ch->getFd(), NULL)==-1){
        LOG_FATAL("epoll_ctl() EPOLL_CTL_DEL error.\n");
    }
    ch->setIsRegist(false);
} 