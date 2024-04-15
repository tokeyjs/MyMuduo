#include"EventLoop.h"

// 创建定时器fd
int createTimerFd(int sec = 30){
  int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC|TFD_NONBLOCK);
  itimerspec timeout;
  memset(&timeout, 0, sizeof(itimerspec));
  timeout.it_value.tv_sec = sec;
  timeout.it_value.tv_nsec = 0;
  timerfd_settime(tfd, 0, &timeout, 0);
  return tfd;
}

EventLoop::EventLoop(bool isMainLoop, int timetvl, int connectionMaxIdle)
    :epoll_(new Epoll),
    tid_(0),
    stop_(false),
    mainLoop_(isMainLoop),
    timetvl_(timetvl),
    connectionMaxIdle_(connectionMaxIdle),
    wakeupFd_(eventfd(0, EFD_NONBLOCK)),
    wakeChannel_(new Channel(wakeupFd_, this)),
    timerFd_(createTimerFd(timetvl_)),
    timerChannel_(new Channel(timerFd_, this)){
        wakeChannel_->setReadCallBack(std::bind(&EventLoop::handleWakeup, this));
        wakeChannel_->enableReading();
        timerChannel_->setReadCallBack(std::bind(&EventLoop::handleTimer, this));
        timerChannel_->enableReading();
}

EventLoop::~EventLoop(){
    stop();
}

// 开始事件循环
void EventLoop::start(){
    tid_ = syscall(SYS_gettid);
    LOG_DEBUG("eventloop[%ld] start.\n", tid_);
    while(!stop_){
        std::vector<Channel*> chs = epoll_->loop();
        TimeStamp tm = TimeStamp::now();
        if(chs.empty()){
            // epoll_wait()超时了
            if(epollTimeOutCallBack_) 
                epollTimeOutCallBack_(this);
            continue;
        }
        for(auto& ch:chs){
            ch->handleEvent(tm);
        }
    }
}

// 停止事件循环
void EventLoop::stop(){
    stop_ = true;
    // 唤醒事件循环
    wakeup();
}

 //更新channel到epoll中
void EventLoop::update(Channel* ch){
    epoll_->updateChannel(ch);
}

 //将ch从epoll中删除
void EventLoop::remove(Channel* ch){
    epoll_->removeChannel(ch);
}

//判断当前线程是否为IO线程（事件循环线程）
bool EventLoop::curThreadIsIOThread(){
    return syscall(SYS_gettid) == tid_;
} 

//向此事件循环添加task（为了让任务在IO线程中执行）
void EventLoop::addTaskToIOThread(Task task){
    {
        std::lock_guard<std::mutex> loc(taskQueueMutex_);
        taskQueue_.push(task);
    }
    // 唤醒事件循环
    wakeup();
}

 //使用eventfd唤醒事件循环
void EventLoop::wakeup(){
  uint64_t val = 1;
  write(wakeupFd_, &val, sizeof(val));
}

 //事件循环被唤醒后的执行函数
void EventLoop::handleWakeup(){
    //从eventfd中读取数据，如果不读取，eventfd的读事件会一直触发
    uint64_t val = 1;
    read(wakeupFd_, &val, sizeof(val)); 
    // 执行队列中的任务
    std::queue<Task> copyTaskQue;
    {
        std::lock_guard<std::mutex> loc(taskQueueMutex_);
        copyTaskQue.swap(taskQueue_); //交换（防止其他工作线程需要向task队列提交任务而被较长时间阻塞住）
    }
    while(!copyTaskQue.empty()){
        Task task = std::move(copyTaskQue.front());
        copyTaskQue.pop();
        task(); //执行任务
    }
}

 //闹钟响铃执行此函数（定时清理空闲连接）
void EventLoop::handleTimer(){
    // 重新计时
    itimerspec timeout;
    memset(&timeout, 0, sizeof(itimerspec));
    timeout.it_value.tv_sec = timetvl_;
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(timerFd_, 0, &timeout, 0);

    if(!mainLoop_){
        // 从事件循环闹钟响了
        TimeStamp nowTime = TimeStamp::now(); //获取当前时间
        {
            std::lock_guard<std::mutex> loc(connMapMutex_);
            for(auto it = conns_.begin(); it!=conns_.end();){
                if(it->second->timeout(nowTime, connectionMaxIdle_)){  // 看看这个连接是否长时间空闲
                    int dfd = it->first;
                    it = conns_.erase(it);
                    timerCallBack_(dfd);
                }else{
                    ++it;
                }
            }
        }

    }
}

 // 设置epoll_wait()超时回调
void EventLoop::setEpollTimeOutCallBack(EpollTimeOutCallBack func){
    epollTimeOutCallBack_ = std::move(func);
}

 //设置定时器回调函数
void EventLoop::setTimerCallBack(TimerCallBack func){
    timerCallBack_ = std::move(func);
}

//将连接添加至map中
void EventLoop::addConnection(SptrTcpConnection conn){
    std::lock_guard<std::mutex> loc(connMapMutex_);
        conns_[conn->getFd()] = conn;
}
//将连接先从map中删除
void EventLoop::removeConnection(SptrTcpConnection conn){
    std::lock_guard<std::mutex> loc(connMapMutex_);
        conns_.erase(conn->getFd());
} 