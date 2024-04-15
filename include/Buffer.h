#pragma once

#include<iostream>
#include<string>
#include<vector>
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>
#include <algorithm>

class Buffer{
public:
    static const size_t kInitialSize = 1024; // 初始缓冲区大小
private:
    std::vector<char> buffer_;  // 存储数据
    int readIndex_; //读取的开始index
    int writeIndex_; //插入数据的开始index
public:
    Buffer(); //构造函数
    int readableBytes() const; //buffer可读数据大小
    int writeableBytes() const; //最大可写入长度（后方）
    int preBytes() const; //前面的空白部分长度
    char* peek() ; // 返回缓冲区可读数据起始地址
    void moveIdxData(); //将数据往前移动，防止前面空间太大，导致不必要的扩容 (当append的数据能够通过前后空位放下时调用)
    char* beginWrite(); //返回可写入起始地址
    void append(const char* data, size_t len); // data里的数据写入缓冲区(不够空间就扩容)

    std::string retrieveAsString(size_t len); //读取缓冲区数据并返回
    std::string retrieveAllAsString(); //读取缓冲区所有数据并返回
    // 从fd上读取数据
    ssize_t readFd(int fd, int* saveErrno);
    // 通过fd发送数据
    ssize_t writeFd(int fd, int* saveErrno);
private:
    char* begin(); //返回数组起始地址
    void makeSpace(size_t len); //扩容
};
