#pragma once

#include <sys/epoll.h> //Linux特有的头文件，提供了epoll API，用于高效的I/O时间通知
//使用协程/boost库

#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()

#include <fstream> //进行文件读写操作
#include <filesystem> //跨平台的文件系统操作接口
#include <cstdlib> //操作环境变量

#include <assert.h> // close()
#include <errno.h>

#include <cassert> //类似于assert.h的头文件，但命名空间在std中
#include <stdexcept> //使用异常机制代替断言，在运行时捕获和处理错误
//static_assert用于编译期断言
#include <system_error> //用于更现代的错误问题处理

#include <vector>

class Epoller
{
public:

     //指定epoll示例能够处理的最大事件数量
    explicit Epoller(int maxEvent = 1024);
    //explicit：禁用隐式类型转换，只允许显式转换，防止构造函数的参数被隐式转换导致
    //只能用于单个参数的构造函数，因为多个参数的构造函数默认不允许使用隐式类型转换

    //释放epoll实例占用的资源，关闭文件描述符，防止资源泄漏
    ~Epoller();

    //将文件描述符fd添加到epoll实例中，并指定关注的事件类型
    bool AddFd(int fd, uint32_t events);

    //在不删除文件描述符的情况下，更新其关注的事件
    bool ModFd(int fd, uint32_t events);

    //从epoll实例中删除文件描述符fd，停止监视指定的文件描述符
    bool DelFd(int fd);

    //等待epoll事件的发生
    int Wait(int timeoutMs = -1);

    //获取第i个发生事件的文件描述符
    int GetEventFd(size_t i) const;
    //const：不希望被修改

    //获取第i个发生事件的事件类型
    uint32_t GetEvents(size_t i) const;
    //与epoll API中的数据类型一致
        
private:

    //epoll实例的文件描述符，用于标识epoll实例，所有的epoll操作都需要该文件描述符
    int epollFd_;

    //用于存储epoll事件的动态数组，保存epoll_wait返回的发生事件的列表
    std::vector<struct epoll_event> events_;    
};