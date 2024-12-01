#include "poll.h"

void Epoll::poll(std::vector<sp_Channel>& req)
{
    int event_count =
        Epoll_wait(epollFd_, &*events_.begin(), events_.size(), EPOLLWAIT_TIME);

    for(int i = 0; i < event_count; ++i)
    {
        int fd = events_[i].data.fd;
        sp_Channel temp = channelMap_[fd];
        temp->setRevents(events_[i].events);
        req.emplace_back(std::move(temp));
    }
    //LOG << "Epoll finished";
}