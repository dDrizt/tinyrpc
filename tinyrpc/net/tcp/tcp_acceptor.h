#pragma once

#include <memory>
#include "tinyrpc/net/tcp/net_addr.h"

namespace tinyrpc {

class TcpAcceptor {
public:
    using s_ptr = std::shared_ptr<TcpAcceptor>;

    TcpAcceptor(NetAddr::s_ptr local_addr);
    ~TcpAcceptor();

    int getListenFd();

    std::pair<int, NetAddr::s_ptr> accept();

private:
    NetAddr::s_ptr local_addr_; // 服务器监听地址，addr -> ip:port

    int listenfd_ {-1}; // 监听套接字

    int family_ {-1};

};

}