#pragma once

#include <functional>
#include <memory>
#include <sys/epoll.h>
#include <unistd.h>



//typedef std::function<void()> EventCallBack;
//定义事件回调函数类型
using event_callback = std::function<void()>;

//封装文件描述符和与之相关的事件，持有事件的回调函数，将事件通知给poller
class channel
{
public:

    explicit channel(int fd);

    ~channel();

    //获取和设置文件描述符
    int get_fd() const;
    void set_fd(int fd);

    //设置事件与就绪事件
    void set_events(uint32_t events);
    void set_revents(uint32_t revents);

    //设置回调函数
    void set_read_callback(event_callback&& read_handler);
    void set_write_callback(event_callback&& write_handler);
    void set_update_handler(event_callback&& update_handler);
    void set_error_handler(event_callback&& error_handler);

    //IO事件回调函数的调⽤接口
    //EventLoop中调⽤Loop开始事件循环，会调⽤Poll得到就绪事件
    //然后依次调⽤此函数处理就绪事件
    void handle_events(); 

    //处理读事件的回调
    void HandleRead(); 

    //处理写事件的回调
    void HandleWrite();

    //处理更新事件的回调
    void HandleUpdate();

    // 处理错误事件的回调
    void HandleError();

    //返回weak_ptr所指向的shared_ptr对象
    std::shared_ptr<http::HttpConnection> holder();
    void set_holder(std::shared_ptr<http::HttpConnection> holder);

    
 

    int& events();
    
    int last_events();
    bool update_last_events();

private:

    //Channel的fd
    int fd_;

    //Channel正在监听的事件（或者说感兴趣的事件）
    uint32_t events_;

    //返回的就绪事件
    uint32_t revents_;

    event_callback read_handler_; 
    event_callback write_handler_;
    event_callback update_handler_;
    event_callback error_handler_;



    //上⼀次事件（主要⽤于记录如果本次事件和上次事件⼀样，就没必要调⽤epoll_mod）
    int last_events_;

    //weak_ptr是⼀个观测者（不会增加或减少引⽤计数）
    //同时也没有重载->，和*等运算符 所以不能直接使⽤
    //可以通过lock函数得到它的shared_ptr（对象没销毁就返回，销毁了就返回空shared_ptr）
    //expired函数判断当前对象是否销毁了
    std::weak_ptr<http::HttpConnection> holder_;

    
};