#pragma once

#include <set>
#include "tinyrpc/net/tcp/tcp_acceptor.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/io_thread_group.h"
#include "tinyrpc/net/eventloop.h"
#include "tinyrpc/net/tcp/tcp_connection.h"

namespace tinyrpc {

class TcpServer {
public:
    TcpServer(NetAddr::s_ptr local_addr);

    ~TcpServer();

    void start();

private:
    void init();

    // 当新客户端连接后执行
    void onAccept();
    
private:
    int client_counts {0};

    TcpAcceptor::s_ptr acceptor_;

    NetAddr::s_ptr local_addr_; // 本地监听地址

    EventLoop* main_event_loop_ {nullptr};  // mainReactor

    IOThreadGroup* io_thread_group_ {nullptr};  // subRector 组

    FdEvent* listen_fd_event_ {nullptr};

    std::set<TcpConnection::s_ptr> client_;
    
};

}