#include "acceptor.hpp"

#include <sys/socket.h>
#include <unistd.h>

Acceptor::Acceptor(eventLoop* loop, const InetAddress &listenAddr)
    : loop_(loop), acceptSocket_(), listening_(false) {

    acceptSocket_.bindAddress(listenAddr.get_sockaddr_in());
    acceptChannel_ = std::make_shared<channel>(acceptSocket_.fd());
    acceptChannel_->set_read_callback([this]{ this->handleRead(); });
}

Acceptor::~Acceptor() {}

void Acceptor::listen() {
    listening_ = true;
    acceptSocket_.listenFD();
    acceptChannel_->set_events(EPOLLIN | EPOLLET);
    loop_->epollerAdd(acceptChannel_);
}

void Acceptor::handleRead() {
    struct sockaddr_in addr;
    int connfd = acceptSocket_.acceptFD(&addr);
    if(connfd >= 0) {
        InetAddress peerAddr(addr);
        if(newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        } else {
            ::close(connfd); // 无回调就关闭连接
        }
    }
}
