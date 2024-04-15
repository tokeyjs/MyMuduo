#pragma once
#include"Channel.h"
#include"TimeStamp.h"
#include"Epoll.h"
#include<functional>
#include<memory>
#include<queue>
#include<mutex>
#include<unordered_map>
#include<atomic>
#include<sys/eventfd.h>
#include<sys/timerfd.h>
#include<syscall.h>
#include"TcpConnection.h"

class Epoll;
class Channel;
class TcpConnection;

class EventLoop{
private:
    using EpollTimeOutCallBack = std::function<void(EventLoop*)>;
    using Task = std::function<void()> ;
    using TimerCallBack = std::function<void(int)>;
    using SptrTcpConnection = std::shared_ptr<TcpConnection>; 

    std::unique_ptr<Epoll> epoll_; // epoll
    pthread_t tid_; //IO线程id
    std::atomic_bool stop_; // 时间循环是否停止
    bool mainLoop_; //当前eventloop是否为主事件循环
    EpollTimeOutCallBack epollTimeOutCallBack_ ;// epoll_wait() 超时回调函数

    std::queue<Task> taskQueue_; //任务队列（事件循环被唤醒后需要执行）
    std::mutex taskQueueMutex_; //任务队列锁
    std::unordered_map<int, SptrTcpConnection> conns_; //存放运行在事件循环的全部connection
    std::mutex connMapMutex_; // map锁

    // 唤醒事件循环的eventfd
    int wakeupFd_; //唤醒事件循环
    std::unique_ptr<Channel> wakeChannel_; 

    // 定时器（定时清理超时连接）
    int timetvl_; //定时器定时时间
    int connectionMaxIdle_; //connection最大空闲时间
    int timerFd_; //定时器fd
    std::unique_ptr<Channel> timerChannel_; 
    TimerCallBack timerCallBack_; //删除tcpserver中长期空闲的connection对象 （传入fd）
    
public:
    EventLoop(bool isMainLoop=true, int timetvl = 10, int connectionMaxIdle = 30);
    ~EventLoop();

    // 开始事件循环
    void start();
    // 停止事件循环
    void stop();

    void update(Channel* ch); //更新channel到epoll中
    void remove(Channel* ch); //将ch从epoll中删除

    bool curThreadIsIOThread(); //判断当前线程是否为IO线程（事件循环线程）

    //向此事件循环添加task（为了让任务在IO线程中执行）
    void addTaskToIOThread(Task task);

    void wakeup(); //使用eventfd唤醒事件循环

    void handleWakeup(); //事件循环被唤醒后的执行函数
    void handleTimer(); //闹钟响铃执行此函数（定时清理空闲连接）

    void setEpollTimeOutCallBack(EpollTimeOutCallBack func); // 设置epoll_wait()超时回调
    void setTimerCallBack(TimerCallBack func); //设置定时器回调函数

    void addConnection(SptrTcpConnection conn); //将连接添加至map中
    void removeConnection(SptrTcpConnection conn); //将连接先从map中删除

};