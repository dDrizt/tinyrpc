#pragma once

#include <memory>
#include "tinyrpc/net/io_thread.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_buffer.h"

namespace tinyrpc {

class TcpConnection {

public:
    using s_ptr = std::shared_ptr<TcpConnection>;

    enum class TcpState : int {
        NotConnected = 1,
        Connected = 2,
        HalfClosing = 3,
        Closed = 4 
    };

public:
    TcpConnection(IOThread* io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr);

    ~TcpConnection();

    void onRead();

    void excute();

    void onWrite();

    void setState(TcpConnection::TcpState state = TcpConnection::TcpState::Connected);

    TcpConnection::TcpState getState();

    void clear();

    // 服务器主动关闭连接
    void shutdown();

private:
    IOThread* io_thread_ {nullptr}; // 代表持有该连接的 IO 线程
    
    NetAddr::s_ptr local_addr_;
    NetAddr::s_ptr peer_addr_;

    TcpBuffer::s_ptr in_buffer_;    // 接收缓冲区
    TcpBuffer::s_ptr out_buffer_;   // 发送缓冲区

    FdEvent* fd_event_ {nullptr};

    TcpState state_;
    
    int fd_ {0};

};

}