#include "epoller.hpp"

//使用初始化列表初始化epoll_fd_
//使用epoll_create1直接设置文件描述符标志close-on-exec
//实现了原子性地操作，避免了文件描述符泄露和竞态条件，简化资源管理
//EPOLL_CLOEXEC：特定于epoll系统调用的标志，当进程创建子进程时，自动关闭此fd
epoller::epoller() : epoll_fd_(epoll_create1(EPOLL_CLOEXEC))
{
    //检查初始化是否成功，若失败则抛出异常
    if (epoll_fd_ < 0)
    {
        throw std::system_error(errno, std::system_category(), "Failed to create epoll instance");
    }
}

//释放资源
epoller::~epoller()
{
    close(epoll_fd_);
}

void epoller::epoll_add(const sp_channel& request, uint32_t events)
{
    epoll_event ev = {};
    ev.events = events;
    ev.data.fd = request->get_fd();

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, request->get_fd(), &ev) < 0)
    {
        throw std::system_error(errno, std::system_category(), "Failed to add channel");
    }

    channel_map_[request->get_fd()] = request;
}

void epoller::epoll_mod(const sp_channel& request, uint32_t events)
{
    epoll_event ev = {};
    ev.events = events;
    ev.data.fd = request->get_fd();

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, request->get_fd(), &ev) < 0)
    {
        throw std::system_error(errno, std::system_category(), "Failed to add channel");
    }

    channel_map_[request->get_fd()] = request;
}

void epoller::epoll_del(const sp_channel& request)
{
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, request->get_fd(), nullptr) < 0)
    {
        throw std::system_error(errno, std::system_category(), "Failed to delete channel");
    }

    channel_map_.erase(request->get_fd());
}

std::vector<sp_channel> epoller::poll(int timeout)
{
    events_.resize(1024);
    int event_count = epoll_wait(epoll_fd_, events_.data(), events_.size(), timeout);
    std::vector<sp_channel> active_channels;

    if (event_count < 0)
    {
        throw std::system_error(errno, std::system_category(), "epoll_wait failed");
    }

    for(int i = 0; i < event_count; i++)
    {
        int fd = events_[i].data.fd;
        auto it = channel_map_.find(fd);
        
        if (it != channel_map_.end())
        {
            it->second->set_revents(events_[i].events);
            active_channels.push_back(it->second);
        }
    }

    return active_channels;

    //LOG << "Epoll finished";
}