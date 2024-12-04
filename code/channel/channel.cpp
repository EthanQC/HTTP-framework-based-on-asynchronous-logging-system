#include "channel.h"

explicit channel::channel(int fd) : fd_(fd), events_(0), revents_(0) {}

channel::~channel()
{
    if (fd_ >= 0)
    {
        close(fd_);
    }
}

int channel::get_fd() const
{
    return fd_;
}

void channel::set_fd(int fd)
{
    fd_ = fd;
}

void channel::set_events(uint32_t events)
{
    events_ = events;
}

void channel::set_revents(uint32_t revents)
{
    revents_ = revents;
}

void channel::set_read_callback(event_callback&& read_handler)
{
    read_handler = std::move(read_handler);
}

void channel::set_write_callback(event_callback&& write_handler)
{
    write_handler = std::move(write_handler);
}

void channel::set_update_handler(event_callback&& update_handler)
{
    update_handler = std::move(update_handler);
}

void channel::set_error_handler(event_callback&& error_handler)
{
    error_handler = std::move(error_handler);
}

//IO事件的回调函数EventLoop中调⽤Loop开始事件循环，会调⽤Poll得到就绪事件
//然后依次调⽤此函数处理就绪事件
void channel::handle_events()
{
    //清空事件状态
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