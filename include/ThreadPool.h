#pragma once

#include<sys/syscall.h>
#include<mutex>
#include<unistd.h>
#include<thread>
#include<condition_variable>
#include<functional>
#include<future>
#include<atomic>
#include<vector>
#include<iostream>
#include<atomic>
#include<queue>
#include<memory>


// 线程池类
class ThreadPool{
public:
    using Task = std::function<void()>; //任务类型
private:
    std::vector<std::thread> threads_; //线程池中的线程
    std::queue<Task> taskQueue_; // 任务队列
    std::mutex mutex_; //队列互斥锁
    std::condition_variable condition_; // 条件变量
    std::atomic_bool stop_; // 线程池开始和结束
public:

    // 在构造函数中将启动threadnum个线程
    ThreadPool(size_t threadnum);

    // 析构函数中停止线程
    ~ThreadPool();

    // 将任务提交到任务队列
    void addTask(Task task);

    void stop(); //线程池停止

    int size(); //返回线程数量

};