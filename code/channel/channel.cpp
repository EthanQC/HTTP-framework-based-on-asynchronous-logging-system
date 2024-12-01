#include "channel.h"

//IO事件的回调函数EventLoop中调⽤Loop开始事件循环，会调⽤Poll得到就绪事件
//然后依次调⽤此函数处理就绪事件
void Channel::HandleEvents()
{
    events_ = 0;

    //触发挂起事件，并且没触发可读事件
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        events_ = 0;
        return;
    }

    //触发错误事件
    if (revents_ & EPOLLERR)
    {
        HandleError();
        events_ = 0;
        return;
    }

    //触发可读事件 | ⾼优先级可读 | 对端（客户端）关闭连接
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        HandleRead();
    }

    //触发可写事件
    if (revents_ & EPOLLOUT)
    {
        HandleWrite();
    }

    //处理更新监听事件（EpollMod）
    HandleUpdate();
}