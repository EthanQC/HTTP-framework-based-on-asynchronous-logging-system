#pragma once

#include <functional>
#include <memory>
#include <sys/epoll.h>



//封装文件描述符和与之相关的事件，持有事件的回调函数，将事件通知给poller
class channel
{
public:
    typedef std::function<void()> EventCallBack;

    channel();
    explicit channel(int fd);

    ~channel();

    //IO事件回调函数的调⽤接口
    //EventLoop中调⽤Loop开始事件循环 会调⽤Poll得到就绪事件
    //然后依次调⽤此函数处理就绪事件
    void HandleEvents(); 

    //处理读事件的回调
    void HandleRead(); 

    //处理写事件的回调
    void HandleWrite();

    //处理更新事件的回调
    void HandleUpdate();

    // 处理错误事件的回调
    void HandleError();

    int get_fd();
    void set_fd(int fd);

    //返回weak_ptr所指向的shared_ptr对象
    std::shared_ptr<http::HttpConnection> holder();
    void set_holder(std::shared_ptr<http::HttpConnection> holder);

    //设置回调函数
    void set_read_handler(EventCallBack&& read_handler);
    void set_write_handler(EventCallBack&& write_handler);
    void set_update_handler(EventCallBack&& update_handler);
    void set_error_handler(EventCallBack&& error_handler);
 
    void set_revents(int revents);
    int& events();
    void set_events(int events);
    int last_events();
    bool update_last_events();

private:

    //Channel的fd
    int fd_;

    //Channel正在监听的事件（或者说感兴趣的时间）
    int events_;

    //返回的就绪事件
    int revents_;

    //上⼀此事件（主要⽤于记录如果本次事件和上次事件⼀样，就没必要调⽤epoll_mod）
    int last_events_;

    //weak_ptr是⼀个观测者（不会增加或减少引⽤计数）
    //同时也没有重载->，和*等运算符 所以不能直接使⽤
    //可以通过lock函数得到它的shared_ptr（对象没销毁就返回，销毁了就返回空shared_ptr）
    //expired函数判断当前对象是否销毁了
    std::weak_ptr<http::HttpConnection> holder_;

    EventCallBack read_handler_; 
    EventCallBack write_handler_;
    EventCallBack update_handler_;
    EventCallBack error_handler_;
};