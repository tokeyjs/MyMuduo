#include<iostream>
#include"TimeStamp.h"
#include"Logger.h"
#include"EchoServer.h"
#include"Buffer.h"

int main(int argc, char* argv[]){
    if(argc!=3){
        std::cout<<"error \nusage: ./server 127.0.0.1 8888"<<std::endl;
        return -1;
    } 
    std::cout<< TimeStamp::now().toString()<<std::endl;
    EchoServer ser(argv[1],atoi(argv[2]), 2, 2);
    ser.Start();
    return 0;
}