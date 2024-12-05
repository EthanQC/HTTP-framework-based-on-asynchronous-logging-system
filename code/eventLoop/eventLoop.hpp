#pragma once

#include <sys/types.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <queue>
#include <cassert>
//#include <cstring>
//#include <errno.h>
#include <atomic>
#include <thread>
#include <mutex>
#include "channel/channel.hpp"
#include "epoller/epoller.hpp"
#include "timer/timer.hpp"

// function是functional中的模板，用来统一封装具有同种返回值、参数类型但不同的可调用对象（函数、lambda等）
// 等同于：typedef std::function<void()> Function;，但用using更现代化
// typedef：cpp11新特性，用于为现有数据类型创建别名
using Function = std::function<void()>;

// 负责整体的事件循环和调度，处理来自poller的活跃事件，调用相应的回调函数
class eventLoop
{
public:
    
    // 初始化poller, event_fd，给event_fd注册到epoll中并注册其事件处理回调
    eventLoop();
    ~eventLoop();

    bool is_in_loop_thread() const;

    // 开始事件循环，调⽤该函数的线程必须是该EventLoop所在线程，也就是Loop函数不能跨线程调⽤
    void loop();

    // 停⽌Loop
    void stopLoop();

    // 如果当前线程就是创建此EventLoop的线程，就调⽤callback（关闭连接EpollDel）
    // 否则就放⼊等待执⾏函数区
    void runInLoop(Function&& func);

    // 把此函数放⼊等待执⾏函数区，如果当前是跨线程，或者正在调⽤等待的函数则唤醒
    void queueInLoop(Function&& func);

    // 把fd和绑定的事件注册到epoll内核事件表
    void epollerAdd(std::shared_ptr<channel> channel, int timeout = 0);

    // 在epoll内核事件表修改fd所绑定的事件
    void epollerMod(std::shared_ptr<channel> channel, int timeout = 0);

    // 从epoll内核事件表中删除fd及其绑定的事件
    void epollerDel(std::shared_ptr<channel> channel);

    // 只关闭连接(此时还可以把缓冲区数据写完再关闭)
    void shutDown(std::shared_ptr<channel> channel);

private:

    // 创建eventfd，类似管道的进程间通信⽅式
    static int createEventfd();

    // eventfd的读回调函数（因为event_fd写了数据，所以触发可读事件，从event_fd读数据）
    void handleRead();

    // eventfd的更新事件回调函数（更新监听事件）
    void handleUpdate();

    // 异步唤醒SubLoop的epoll_wait（向event_fd中写⼊数据）
    void wakeUp();

    // 执⾏正在等待的函数（SubLoop注册EpollAdd连接套接字以及绑定事件的函数）
    void performPendingFunctions();

    // io多路复⽤分发器
    std::shared_ptr<epoller> epoller_;

    // ⽤于异步唤醒SubLoop的Loop函数中的Poll（epoll_wait因为还没有注册fd会⼀直阻塞）
    int event_fd_;

    // ⽤于异步唤醒的channel
    std::shared_ptr<channel> wakeup_channel_;

    // 线程id
    std::thread::id thread_id_;

    // 互斥锁保护pending_functions_
    std::mutex mutex_;

    // 正在等待处理的函数
    std::vector<Function> pending_functions_;

    // 定时器
    timer timer_;

    std::atomic<bool> is_stop_; // 是否停⽌事件循环
    std::atomic<bool> is_looping_; // 是否正在事件循环
    std::atomic<bool> is_event_handling_; // 是否正在处理事件
    std::atomic<bool> is_calling_pending_functions_; // 是否正在调用等待处理的函数
};