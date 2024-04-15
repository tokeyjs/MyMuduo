#pragma once
#include<string>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/tcp.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include"InetAddress.h"

// 创建非阻塞的socket服务端fd
int createNonBlockingListenFD();

class InetAddress;

// 封装socket fd
class Socket{
public:
    explicit Socket(int sockfd) // 已经初始化好的一个fd
        : sockfd_(sockfd)
    {}

    ~Socket();

    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress &localaddr);
    void listen(int nn = 1024);
    int accept(InetAddress &peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

    void setIp(std::string ip);
    void setPort(uint16_t port);
    std::string getIp();
    uint16_t getPort();
private:
    const int sockfd_;
    std::string ip_; //ip地址
    uint16_t port_; //port
};