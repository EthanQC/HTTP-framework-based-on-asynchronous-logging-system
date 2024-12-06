#include "channel.hpp"

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

uint32_t channel::get_events() const
{
    return events_;
}

uint32_t channel::get_revents() const
{
    return revents_;
}

void channel::set_read_callback(event_callback&& read_handler)
{
    read_handler_ = std::move(read_handler);
}

void channel::set_write_callback(event_callback&& write_handler)
{
    write_handler_ = std::move(write_handler);
}

void channel::set_update_handler(event_callback&& update_handler)
{
    update_handler_ = std::move(update_handler);
}

void channel::set_error_handler(event_callback&& error_handler)
{
    error_handler_ = std::move(error_handler);
}

// IO事件的回调函数EventLoop中调⽤Loop开始事件循环，会调⽤Poll得到就绪事件
// 然后依次调⽤此函数处理就绪事件
void channel::handle_events()
{
    // 处理挂起事件（EPOLLHUP）：
    // 如果检测到对端关闭连接，但没有可读事件（EPOLLIN），调用错误回调
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (error_handler_) handle_error();
        return;
    }

    // 处理错误事件（EPOLLERR）：
    // 如果发生错误事件，调用错误回调
    if (revents_ & EPOLLERR)
    {
        if (error_handler_) handle_error();
        return;
    }

    // 处理可读事件（EPOLLIN、EPOLLPRI、EPOLLRDHUP）
    // 包括普通读事件、高优先级读事件、对端半关闭事件
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        if (read_handler_) handle_read();
    }

    // 处理可写事件（EPOLLOUT）
    // 如果触发了写事件，调用写回调
    if (revents_ & EPOLLOUT)
    {
        if (write_handler_) handle_write();
    }

    // 处理更新事件（例如监听事件的修改，EpollMod）
    // 可选逻辑，通常用于动态更新监听事件
    if (revents_ & (EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP))
    {
        if (update_handler_) handle_update();
    }

    // 重置返回的事件
    revents_ = 0;
}

void channel::handle_read()
{
    read_handler_();
}

void channel::handle_write()
{
    write_handler_();
}

void channel::handle_update()
{
    update_handler_();
}

void channel::handle_error()
{
    error_handler_();
}