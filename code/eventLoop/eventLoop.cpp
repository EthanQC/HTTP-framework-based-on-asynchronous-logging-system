#include "eventLoop.hpp"

eventLoop::eventLoop() : 
    epoller_(std::make_shared<epoller>()),
    event_fd_(createEventfd()),
    wakeup_channel_(std::make_shared<channel>(event_fd_)),
    thread_id_(std::this_thread::get_id()),
    is_stop_(false),
    is_looping_(false),
    is_event_handling_(false),
    is_calling_pending_functions_(false)
{
    // 设置wakeup_channel_的事件和回调
    wakeup_channel_->set_events(EPOLLIN | EPOLLET);
    wakeup_channel_->set_read_callback([this]() {this->handleRead();});
    epoller_->epoll_add(wakeup_channel_, wakeup_channel_->get_events());

    // 添加定时器的文件描述符到epoll中（如果定时器模块需要）
    // 这里假设HeapTimer不需要单独的文件描述符
}

eventLoop::~eventLoop()
{
    wakeup_channel_->set_fd(-1);
    close(event_fd_);
}

bool eventLoop::is_in_loop_thread() const
{
    // 使用std::this_thread::get_id()来获取当前线程ID
    // 为简化，假设单线程
    return thread_id_ == std::this_thread::get_id();
}

void eventLoop::loop()
{
    // 开始事件循环，调⽤该函数的线程必须是该EventLoop所在线程
    assert(!is_looping_);
    assert(is_in_loop_thread());
    is_looping_ = true;
    is_stop_ = false;

    while (!is_stop_)
    {
        try
        {
            // 获取下一个定时器的超时时间
            int timeout = timer_.getNextTick();

            // 1.epoll_wait阻塞，等待就绪事件
            auto ready_channels = epoller_->epoll(timeout);
            is_event_handling_ = true;

            // 2.处理每个就绪事件（不同channel绑定了不同的callback）
            for (auto& channel : ready_channels)
            {
                channel->handle_events();
            }
            is_event_handling_ = false;

            // 3.执⾏正在等待的函数（fd注册到epoll内核事件表）
            performPendingFunctions();

            // 4.处理超时事件，到期了就从定时器⼩根堆中删除（定时器析构会EpollDel掉fd）
            timer_.tick();
        }
        catch(const std::exception& e)
        {
            std::cerr << "Exception in Loop:" << e.what() << std::endl;
        }
    }

    is_looping_ = false;
}

void eventLoop::stopLoop()
{
    is_stop_ = true;
    wakeUp();
}

void eventLoop::runInLoop(Function&& func)
{
    if (is_in_loop_thread())
    {
        func();
    }
    else
    {
        queueInLoop(std::move(func));
    }
}

void eventLoop::queueInLoop(Function&& func)
{
    std::lock_guard<std::mutex> lock(mutex_);
    pending_functions_.emplace_back(std::move(func));
    wakeUp();
}

void eventLoop::epollerAdd(std::shared_ptr<channel> channel, int timeout = 0)
{
    epoller_->epoll_add(channel, channel->get_events());
}

void eventLoop::epollerMod(std::shared_ptr<channel> channel, int timeout = 0)
{
    epoller_->epoll_mod(channel, channel->get_events());
}

void eventLoop::epollerDel(std::shared_ptr<channel> channel)
{
    epoller_->epoll_del(channel);
}

void eventLoop::shutDown(std::shared_ptr<channel> channel)
{
    if (channel)
    {
        ::shutdown(channel->get_fd(), SHUT_RDWR);
    }
}

// 创建eventfd，设置为非阻塞和关闭时自动关闭
int eventLoop::createEventfd()
{
    int event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);

    if (event_fd < 0)
    {
        // 使用 std::system_error 抛出异常，传递 errno 和相关信息
        throw std::system_error(errno, std::generic_category(), "Failed to create eventfd");
    }

    return event_fd;
}

void eventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(event_fd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        std::cerr << "handleRead reads " << n << " bytes instead of 8" << std::endl;
    }
}

void eventLoop::handleUpdate()
{
    // 实现更新逻辑，如果需要的话
}

void eventLoop::wakeUp()
{
    uint64_t one = 1;
    ssize_t n = write(event_fd_, &one, sizeof(one));

    if (n != sizeof(one))
    {
        std::cerr << "wakeUp writes " << n << " bytes instead of 8" << std::endl;
    }
}

void eventLoop::performPendingFunctions()
{
    std::vector<Function> funcs;
    std::lock_guard<std::mutex> lock(mutex_);
    funcs.swap(pending_functions_);

    for (auto& func : funcs)
    {
        func();
    }
}