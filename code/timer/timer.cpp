#include "timer.hpp"

void timer::siftup_(size_t i)
{
    assert(i < heap_.size());
    
    while (i > 0)
    {
        size_t parent = (i - 1) / 2;

        if (heap_[parent] < heap_[i])
        {
            break;
        }
        
        swapNode_(i, parent);
        i = parent;
    }
}

void timer::swapNode_(size_t i, size_t j)
{
    assert(i < heap_.size() && j < heap_.size());

    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
} 

bool timer::siftdown_(size_t index, size_t n)
{
    assert(index < heap_.size());

    size_t i = index;
    size_t j = i * 2 + 1;

    while(j < n)
    {
        if(j + 1 < n && heap_[j + 1] < heap_[j]) j++;
        if(heap_[i] < heap_[j]) break;
        swapNode_(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

void timer::add(int id, int timeout, const timeoutCallback& cb)
{
    assert(id >= 0);
    std::lock_guard<std::mutex> lock(mutex_);

    if(ref_.count(id) == 0)
    {
        // 新节点：堆尾插入，调整堆
        size_t i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, clock::now() + ms(timeout), cb});
        siftup_(i);
    } 
    else
    {
        // 已有结点：调整堆
        size_t i = ref_[id];
        heap_[i].expires = clock::now() + ms(timeout);
        heap_[i].cb = cb;
        if (!siftdown_(i, heap_.size()))
        {
            siftup_(i);
        }
    }
}

void timer::doWork(int id)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // 删除指定id结点，并触发回调函数
    if(heap_.empty() || ref_.count(id) == 0) return;

    size_t i = ref_[id];
    timerNode node = heap_[i];
    node.cb();
    del_(i);
}

void timer::del_(size_t index)
{
    // 删除指定位置的结点
    assert(!heap_.empty() && index < heap_.size());

    // 将要删除的结点换到队尾，然后调整堆
    size_t i = index;
    size_t n = heap_.size() - 1;

    if(i < n)
    {
        swapNode_(i, n);
        heap_.pop_back();
        ref_.erase(heap_[i].id);

        if (!siftdown_(i, heap_.size()))
        {
            siftup_(i);
        }
    }
    else
    {
        // 队尾元素删除
        ref_.erase(heap_.back().id);
        heap_.pop_back();
    }
}

void timer::tick()
{
    // 清除超时结点
    if(heap_.empty()) return;

    while (!heap_.empty())
    {
        timerNode node = heap_.front();
        auto now = clock::now();

        if(std::chrono::duration_cast<ms>(node.expires - now).count() > 0) break; 

        node.cb();
        pop();
    }
}

void timer::pop()
{
    del_(0);
}

void timer::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    ref_.clear();
    heap_.clear();
}

int timer::getNextTick()
{
    std::lock_guard<std::mutex> lock(mutex_);
    tick();

    if (!heap_.empty()) return -1;

    auto now = clock::now();
    auto diff = std::chrono::duration_cast<ms>(heap_.front().expires - now).count();
    return diff > 0 ? static_cast<int>(diff) : 0;
}