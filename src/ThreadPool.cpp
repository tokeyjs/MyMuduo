#include"ThreadPool.h"


// 在构造函数中将启动threadnum个线程
ThreadPool::ThreadPool(size_t threadnum):stop_(false){
    // 创建线程
    for(int i=0;i<threadnum;i++){
        threads_.emplace_back(std::thread([this](){
            // 每个线程都需要做得循环 (消费者)
            Task task;
            printf("create thread(%ld).\n", syscall(SYS_gettid));
            while(!this->stop_){
                {
                    std::unique_lock<std::mutex> loc(this->mutex_);
                    this->condition_.wait(loc, [this]()->bool{
                        return this->stop_||(!this->stop_&&!this->taskQueue_.empty());
                    });
                    if(this->stop_&&this->taskQueue_.empty()) return;
                    task = std::move(this->taskQueue_.front());
                    this->taskQueue_.pop();
                }
                task();// 执行任务
            }
        }));
    }
}

void ThreadPool::stop(){
    if(stop_) return;
    // 设置停止标志
    stop_ = true;
    condition_.notify_all(); //唤醒所有线程
    // 等待所有线程退出
    for(auto& th:threads_)
        th.join();
    printf("threadpoll exit.\n");
}

// 析构函数中停止线程
ThreadPool::~ThreadPool(){
    stop(); //停止所有线程
}

// 将任务提交到任务队列
void ThreadPool::addTask(Task task){
    if(stop_) return;
    // (生产者)
    std::unique_lock<std::mutex> loc(this->mutex_); // 可以在指定时间内尝试获取锁，超时任务提交失败
    if(!stop_) taskQueue_.emplace(task);
    // 唤醒因为没任务而睡眠的线程
    this->condition_.notify_one();
}

// 返回线程池线程数量
int ThreadPool::size(){
    return threads_.size();
}