#pragma once

#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>

// 实现思路：
// Buffer模块是对读写缓冲区的封装，用于非阻塞读写，从fd中读取数据放入input buffer，
// 写数据时将数据从output buffer中发送。当fd可写时尝试发送尽可能多的数据。
// 可以使用std::vector<char>或std::string作为底层存储，提供append()、retrieve()等接口

// 缓存区（Buffer）：
// 在网络通信中，数据收发过程是分块且分时进行的。
// 为了高效处理I/O事件，需要对数据进行缓冲。
// 缓存区（如环形缓冲区）可以用来存放暂时接收但未处理完的数据，
// 或者在发送前将数据拼装好，从而减少系统调用次数，提高吞吐率。

class Buffer {
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t WritableBytes() const;       
    size_t ReadableBytes() const ;
    size_t PrependableBytes() const;

    const char* Peek() const;
    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);

    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);

    void RetrieveAll() ;
    std::string RetrieveAllToStr();

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr_();
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len);

    std::vector<char> buffer_;
    std::atomic<std::size_t> readPos_;
    std::atomic<std::size_t> writePos_;
};