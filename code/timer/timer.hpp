#pragma once

#include <queue>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <functional> 
#include <cassert>
#include <chrono>
#include <mutex>

#include "../log/log.h"

// muduo的TimerQueue核心思想：
// 使用timerfd_create创建一个timerfd，注册到epoll中。
// 当定时时间到达时，timerfd会产生可读事件，从而唤醒loop
// loop在对应的handle函数中执行超时任务的回调函数。
// Muduo的TimerQueue使用最小堆或有序容器存储定时器事件。
// 在定时器超时后，通过timerfd通知EventLoop进行回调处理。
// 超时关闭逻辑：
// 可以在TcpConnection或类似连接管理处添加定时器
// 当连接在一定时间内无数据读写，则关闭连接。添加定时器时关联fd与超时事件，到期删除连接。
// 下面会在代码中演示TimerQueue和Timer类的修改：
// 使用timerfd取代原先的纯软件定时轮询方式
// 在eventLoop中与epoller集成，让定时事件像IO事件一样统一处理。

// 清理长时间未活动的连接（超时处理），超时关闭空闲连接
// 定期输出统计日志
// 刷新缓存、重载配置

// 定义超时回调函数类型
using timeoutCallback = std::function<void()>;
using clock = std::chrono::steady_clock;
using ms = std::chrono::milliseconds;
using timeStamp = clock::time_point;

// 定时器节点结构体
struct timerNode
{
    int id;
    timeStamp expires;
    timeoutCallback cb;

    bool operator<(const timerNode& t) const
    {
        // 优先级队列按小顶堆排序，所以这里反向比较
        return expires > t.expires;
    }
};

// 定时器堆管理类
class timer
{
public:

    timer() = default;

    ~timer() { clear(); }

    // 禁用拷贝和赋值
    timer(const timer&) = delete;
    timer& operator=(const timer&) = delete;

    // 添加或修改定时器
    void add(int id, int timeout, const timeoutCallback& cb);

    // 删除定时器并执行回调
    void doWork(int id);

    // 清除所有定时器
    void clear();

    // 处理超时定时器
    void tick();

    void pop();

    // 获取下一个定时器的超时时间（毫秒）
    int getNextTick();

private:

    void del_(size_t i);
    
    void siftup_(size_t i);

    bool siftdown_(size_t index, size_t n);

    void swapNode_(size_t i, size_t j);

    std::vector<timerNode> heap_;

    std::unordered_map<int, size_t> ref_;

    // 保护heap_和ref_
    std::mutex mutex_;
};