#pragma once
// 容器
#include <vector>
#include <unordered_map>
// 标准异常库
#include <stdexcept>
#include <system_error>
#include "channel/channel.hpp"

using sp_channel = std::shared_ptr<channel>;

// 对epll系统调用的封装，负责管理文件描述符和它们的I/O事件
class epoller
{
public:

    epoller();

    ~epoller();

    // 添加事件
    void epoll_add(const sp_channel& request, uint32_t events);

    // 修改监听事件
    void epoll_mod(const sp_channel& request, uint32_t events);

    // 删除事件
    void epoll_del(const sp_channel& request);

    // 获取发生事件的channel列表
    std::vector<sp_channel> epoll(int timeout);

private:

    // 用epoll_create方法返回的epoll句柄
    int epoll_fd_;

    // 存储epoll_wait返回的活跃事件
    std::vector<epoll_event> events_;

    // 保存fd到channel的映射，用于快速查找
    std::unordered_map<int, sp_channel> channel_map_;

    // 定义最大事件数量
    static const int MAX_EVENTS = 1024;
};