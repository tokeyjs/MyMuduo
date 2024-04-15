#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<time.h>
#include"TimeStamp.h"

void deal_1(int sockfd);
void deal_2(int sockfd);

int NMSG = 10000;

// 网络通信客户端程序
int main(int argc, char *argv[]){
    if(argc!=3){
        printf("usage:./client ip port\n");
        printf("example:./client 172.30.88.165 5085\n");
        return -1;
    }


    int sockfd;
    struct sockaddr_in servaddr;
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0))<0){
        printf("socket() failed.\n");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    if((connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)))!=0){
        printf("connect(%s,%s) failed.\n", argv[1], argv[2]);
        close(sockfd);
        return -1;
    }

    // printf("connect ok!\n");

    TimeStamp n1 = TimeStamp::now();

    deal_2(sockfd);

    TimeStamp n2 = TimeStamp::now();
    // sleep(60);
    printf("%d条数据，用时:%dms", NMSG, n2.sub(n1).toMilSec());

    return 0;
}




//=======发送数据与接收数据策略=====

//-> 一收一发（键盘输入）
void deal_1(int sockfd){
    // 收发消息
    char buf[1024];

    for(;;){
        memset(buf,0,sizeof(buf));
        printf("client:");
        scanf("%s", buf);

        // 发送数据给服务端
        if(send(sockfd, buf, strlen(buf), 0)<=0){
            printf("send falied.\n");
            close(sockfd);
            return ;
        }

        // 接收服务端数据
        memset(buf, 0, sizeof(buf));
        if(recv(sockfd, buf, sizeof(buf), 0)<=0){
            printf("recv falied.\n");
            close(sockfd);
            return ;
        }

        printf("server:%s\n", buf);
    }
}


//-> 一收一发（自动输入）
void deal_2(int sockfd){
    char buffer[1024];
    for(int i=0;i<NMSG;i++){
        memset(buffer, 0,sizeof(buffer));
        sprintf(buffer, "this is the %d goods.", i);
        char tmpbuf[1024];
        memset(tmpbuf, 0, sizeof(tmpbuf));
        int len = strlen(buffer);
        // memcpy(tmpbuf, &len, 4);
        // memcpy(tmpbuf+4, buffer, len);
        memcpy(tmpbuf, buffer, len);

        // if(send(sockfd, tmpbuf, len+4, 0)<=0){
        if(send(sockfd, tmpbuf, len, 0)<=0){
            printf("send() error.\n");
            close(sockfd);
            return ;
        }
        // 收
        // recv(sockfd, &len, 4, 0);
        memset(buffer,0, sizeof(buffer));

        if(recv(sockfd, buffer, 1024, 0)<=0){
            printf("recv() error.\n");
            close(sockfd);
            return;
        }
        // printf("recv:%s\n", buffer);

    }

    // for(int i=0;i<300;i++){
    //     int len;
    //     recv(sockfd, &len, 4, 0);
    //     memset(buffer,0, sizeof(buffer));

    //     if(recv(sockfd, buffer,len, 0)<=0){
    //         printf("recv() error.\n");
    //         close(sockfd);
    //         return;
    //     }
    //     printf("recv:%s\n", buffer);
    // }
}

