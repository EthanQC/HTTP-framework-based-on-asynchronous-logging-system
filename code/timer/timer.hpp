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