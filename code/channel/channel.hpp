#pragma once

// 标准可调用模板
#include <functional>
// 智能指针
#include <memory>
// Linux的epoll头文件
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>

//定义事件回调函数类型
using event_callback = std::function<void()>;

//封装文件描述符和与之相关的事件，持有事件的回调函数，将事件通知给poller
class channel
{
public:

    explicit channel(int fd);

    ~channel();

    // 获取和设置文件描述符
    int get_fd() const;
    void set_fd(int fd);

    // 设置和获取事件与就绪事件
    void set_events(uint32_t events);
    void set_revents(uint32_t revents);
    uint32_t get_events() const;
    uint32_t get_revents() const;

    // 设置回调函数
    void set_read_callback(event_callback&& read_handler);
    void set_write_callback(event_callback&& write_handler);
    void set_update_handler(event_callback&& update_handler);
    void set_error_handler(event_callback&& error_handler);

    // IO事件回调函数的调⽤接口
    // EventLoop中调⽤Loop开始事件循环，会调⽤Poll得到就绪事件
    // 然后依次调⽤此函数处理就绪事件
    void handle_events(); 

    // 处理读事件的回调
    void handle_read(); 

    // 处理写事件的回调
    void handle_write();

    // 处理更新事件的回调
    void handle_update();

    // 处理错误事件的回调
    void handle_error();

private:

    // Channel的fd
    int fd_;

    // Channel正在监听的事件（或者说感兴趣的事件）
    uint32_t events_;

    // 返回的就绪事件
    uint32_t revents_;

    event_callback read_handler_; 
    event_callback write_handler_;
    event_callback update_handler_;
    event_callback error_handler_;
};