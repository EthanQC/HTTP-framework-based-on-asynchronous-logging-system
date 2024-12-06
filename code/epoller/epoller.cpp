#include "epoller.hpp"

// 使用初始化列表初始化epoll_fd_
// 使用epoll_create1直接设置文件描述符标志close-on-exec
// 实现了原子性地操作，避免了文件描述符泄露和竞态条件，简化资源管理
// EPOLL_CLOEXEC：特定于epoll系统调用的标志，当进程创建子进程时，自动关闭此fd
epoller::epoller() : epoll_fd_(epoll_create1(EPOLL_CLOEXEC)), events_(MAX_EVENTS)
{
    // 检查初始化是否成功，若失败则抛出异常
    if (epoll_fd_ < 0)
    {
        throw std::system_error(errno, std::system_category(), "Failed to create epoll instance");
    }
}

// 释放资源
epoller::~epoller()
{
    if (epoll_fd_ >= 0)
    {
        close(epoll_fd_);
    }
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

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, request->get_fd(), &ev) < 0)
    {
        throw std::system_error(errno, std::system_category(), "Failed to add channel");
    }

    // 确保channel_map中存在该fd
    auto it = channel_map_.find(request->get_fd());
    if (it != channel_map_.end())
    {
        it->second = request;
    }
    else
    {
        channel_map_[request->get_fd()] = request;
    }
}

void epoller::epoll_del(const sp_channel& request)
{
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, request->get_fd(), nullptr) < 0)
    {
        throw std::system_error(errno, std::system_category(), "Failed to delete channel");
    }

    channel_map_.erase(request->get_fd());
}

std::vector<sp_channel> epoller::epoll(int timeout)
{
    int event_count = epoll_wait(epoll_fd_, events_.data(), events_.size(), timeout);
    std::vector<sp_channel> active_channels;

    if (event_count < 0)
    {
        if (errno != EINTR)
        {
            throw std::system_error(errno, std::system_category(), "epoll_wait failed");
        }
        // 如果被信号中断，返回空列表
        return active_channels;
    }

    for(int i = 0; i < event_count; i++)
    {
        int fd = events_[i].data.fd;
        uint32_t ev = events_[i].events;
        auto it = channel_map_.find(fd);
        
        if (it != channel_map_.end())
        {
            it->second->set_revents(events_[i].events);
            active_channels.push_back(it->second);
        }
        else
        {
            std::cerr << "No channel found for fd: " << fd << std::endl;
        }
    }

    return active_channels;
}