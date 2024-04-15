# 基于C++11高性能网络库的设计与实现（仿muduo）

## 1.项目概述

本项目基于C++11开发了一款高性能网络库，模仿muduo库设计理念，采用one loop per thread + thread pool模型相结合，利用non-block IO与IO多路复用技术(epoll)实现多Reactor多线程结构。项目重点实现了Channel、Epoll、EventLoop、Acceptor、Buffer、TcpConnection、TcpServer、ThreadPool模块


## 2.项目实现

使用eventfd唤醒eventloop，保证高效事件响应；应用timerfd定期清除空闲连接，实现资源优化；实现类muduo库Buffer缓冲区类；使用线程池处理客户端请求，借助智能指针优化内存管理，广泛使用函数回调增强库的灵活性与易用性。


## 3.成果展示

基于此网络库构建的EchoServer，在2核1GB机器上进行了性能测试。当面对100个客户端并发发送100万条数据时，服务器在11秒内顺利完成处理，平均处理速率达到每秒9万余条数据，展现出了较高的并发处理效能。

## 其他
时间轮管理定时任务 - 待实现
代码优化 - 待实现
......

## 特别感谢
《Linux多线程服务端编程》-陈硕
