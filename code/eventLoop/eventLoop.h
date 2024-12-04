#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <sys/types.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>
#include <thread>
#include <mutex>

#include "channel/channel.h"

//负责整体的事件循环和调度，处理来自poller的活跃事件，调用相应的回调函数
class eventLoop
{
public:

    typedef std::function<void()> Function;
    
    //初始化poller, event_fd，给event_fd注册到epoll中并注册其事件处理回调
    eventLoop();
    ~eventLoop();

    //开始事件循环 调⽤该函数的线程必须是该EventLoop所在线程，也就是Loop函数不能跨线程调⽤
    void Loop();

    //停⽌Loop
    void StopLoop();

    //如果当前线程就是创建此EventLoop的线程，就调⽤callback（关闭连接EpollDel）
    //否则就放⼊等待执⾏函数区
    void RunInLoop(Function&& func);

    //把此函数放⼊等待执⾏函数区，如果当前是跨线程，或者正在调⽤等待的函数则唤醒
    void QueueInLoop(Function&& func);

    //把fd和绑定的事件注册到epoll内核事件表
    void PollerAdd(std::shared_ptr<channel> channel, int timeout = 0);

    //在epoll内核事件表修改fd所绑定的事件
    void PollerMod(std::shared_ptr<channel> channel, int timeout = 0);

    //从epoll内核事件表中删除fd及其绑定的事件
    void PollerDel(std::shared_ptr<channel> channel);

    //只关闭连接(此时还可以把缓冲区数据写完再关闭)
    void ShutDown(std::shared_ptr<channel> channel);
    bool is_in_loop_thread();

private:

    //创建eventfd，类似管道的进程间通信⽅式
    static int CreateEventfd();

    //eventfd的读回调函数（因为event_fd写了数据，所以触发可读事件，从event_fd读数据）
    void HandleRead();

    //eventfd的更新事件回调函数（更新监听事件）
    void HandleUpdate();

    //异步唤醒SubLoop的epoll_wait（向event_fd中写⼊数据）
    void WakeUp();

    // 执⾏正在等待的函数（SubLoop注册EpollAdd连接套接字以及绑定事件的函数）
    void PerformPendingFunctions();

    //io多路复⽤分发器
    std::shared_ptr<poller> poller_;

    //⽤于异步唤醒SubLoop的Loop函数中的Poll（epoll_wait因为还没有注册fd会⼀直阻塞）
    int event_fd_;

    //⽤于异步唤醒的channel
    std::shared_ptr<channel> wakeup_channel_;

    //线程id
    pid_t thread_id_;

    mutable locker::MutexLock mutex_;

    //正在等待处理的函数
    std::vector<Function> pending_functions_;

    bool is_stop_; //是否停⽌事件循环
    bool is_looping_; //是否正在事件循环
    bool is_event_handling_; //是否正在处理事件
    bool is_calling_pending_functions_; //是否正在调用等待处理的函数
};
