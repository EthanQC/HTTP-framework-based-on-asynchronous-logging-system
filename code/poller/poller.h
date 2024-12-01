#pragma once

#include <vector>
#include <unordered_map>




class Epoll
{
public:

    Epoll();
    ~Epoll();
    void epoll_add(const sp_Channel& request);
    void epoll_mod(const sp_Channel& request);
    void epoll_del(const sp_Channel& request);
    void poll(std::vector<sp_Channel>& req);

private:

    int epollFd_;

    //epoll_wait()返回的活动事件都放在这个数组⾥
    std::vector<epoll_event> events_;
    std::unordered_map<int, sp_Channel> channelMap_;
};