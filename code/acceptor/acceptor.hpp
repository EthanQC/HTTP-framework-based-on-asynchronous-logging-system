#pragma once

#include <functional>

#include <eventLoop/eventLoop.hpp>
#include <channel/channel.hpp>

// 实现思路：
// Acceptor主要用于监听新连接事件
// 内部会创建一个监听套接字（listenfd），将其注册到epoller
// 当有新客户端连接到来时触发读事件，Acceptor的回调负责accept()新连接
// 然后将新连接fd注册到EventLoop中管理。
// Acceptor需要绑定一个EventLoop（通常是主线程的main reactor）



class Acceptor {
public:
    using NewConnectionCallback = std::function<void (int sockfd, const InetAddress &)>;

    Acceptor(eventLoop* loop, const InetAddress &listenAddr);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb) { newConnectionCallback_ = cb; }
    void listen();
    bool isListening() const { return listening_; }

private:
    void handleRead();

    eventLoop* loop_;
    Socket acceptSocket_;
    std::shared_ptr<channel> acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
};
