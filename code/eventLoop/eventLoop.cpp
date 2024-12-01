#include "eventLoop.h"

void EventLoop::Loop()
{
    //开始事件循环，调⽤该函数的线程必须是该EventLoop所在线程
    assert(!is_looping_);
    assert(is_in_loop_thread());
    is_looping_ = true;
    is_stop_ = false;

    while (!is_stop_)
    {
        //1.epoll_wait阻塞,等待就绪事件
        auto ready_channels = poller_->Poll();
        is_event_handling_ = true;

        //2.处理每个就绪事件（不同channel绑定了不同的callback）
        for (auto& channel : ready_channels)
        {
            channel->HandleEvents();
        }
        is_event_handling_ = false;

        //3.执⾏正在等待的函数（fd注册到epoll内核事件表）
        PerformPendingFunctions();

        //4.处理超时事件，到期了就从定时器⼩根堆中删除（定时器析构会EpollDel掉fd）
        poller_->HandleExpire();
    }

    is_looping_ = false;
}