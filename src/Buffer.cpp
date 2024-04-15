#include"Buffer.h"

 //构造函数
Buffer::Buffer()
    :buffer_(kInitialSize),
    readIndex_(0),
    writeIndex_(0){

}

 //buffer可读数据大小
int Buffer::readableBytes() const{
    return writeIndex_-readIndex_;
}

 //最大可写入长度（后方）
int Buffer::writeableBytes() const{
    return buffer_.size()-writeIndex_;
}

//前面的空白部分长度
int Buffer::preBytes() const{
    return readIndex_; 
} 

 // 返回缓冲区可读数据起始地址
char* Buffer::peek() {
    return begin() + readIndex_;
}

//将数据往前移动，防止前面空间太大，导致不必要的扩容 (当append的数据能够通过前后空位放下时调用)
void Buffer::moveIdxData(){
    int idx = 0, copyIdx = readIndex_;
    while(idx<readableBytes()){
        buffer_[idx++] = buffer_[copyIdx++];
    }
    readIndex_ = 0;
    writeIndex_ = idx;
} 

//返回可写入起始地址
char* Buffer::beginWrite(){
    return begin()+writeIndex_;
} 

 // data里的数据写入缓冲区(不够空间就扩容)
void Buffer::append(const char* data, size_t len){
    if(writeableBytes()>len){ // 后方容量足够
    }else if(writeableBytes()+preBytes()>len){ // 前方与后方剩余空间足够，需要移动数据再写入
        moveIdxData();
    }else{ // 位置不够，进行扩容
        makeSpace(len-writeableBytes()+1);
    }
    // 数据写入
    std::copy(data, data+len, beginWrite());
    // 移动
    writeIndex_+=len;
}

//读取缓冲区数据并返回
std::string Buffer::retrieveAsString(size_t len){
    if(len>=readableBytes()) return retrieveAllAsString();
    std::string strdata(peek(), len);
    readIndex_+=len;
    return strdata;
} 

//读取缓冲区所有数据并返回
std::string Buffer::retrieveAllAsString(){
    if(readableBytes()<=0) return "";
    std::string strdata(peek(), readableBytes());
    readIndex_=0;
    writeIndex_=0;
    return strdata;
} 

// 从fd中读取数据
ssize_t Buffer::readFd(int fd, int* saveErrno){
    char extrabuf[65536]{0}; // 栈上的内存空间  64K
    struct iovec vec[2];
    const size_t writable = writeableBytes()-1; // 这是Buffer底层缓冲区剩余的可写空间大小
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0){
        *saveErrno = errno;
    }
    else if (n <= writable){ // Buffer的可写缓冲区已经够存储读出来的数据了
        writeIndex_ += n;
    }
    else{ // extrabuf里面也写入了数据 
        writeIndex_ = buffer_.size()-1;
        append(extrabuf, n - writable);  // writerIndex_开始写 n - writable大小的数据
    }
    return n;
}

// 向一个fd写入数据
ssize_t Buffer::writeFd(int fd, int* saveErrno){
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0){
        *saveErrno = errno;
    }else{
        readIndex_+=n;
    }
    return n;
}


 //返回数组起始地址
char* Buffer::begin(){
    return &*(buffer_.begin());
}

//扩容
void Buffer::makeSpace(size_t len){
    buffer_.resize(buffer_.size()+len);
}