#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/net/tcp/tcp_server.h"
#include "tinyrpc/common/log.h"

namespace tinyrpc {

TcpServer::TcpServer(NetAddr::s_ptr local_addr) : local_addr_(local_addr) {

    init();

    INFOLOG("tinyRPC TcpServer listen success on [%s]", local_addr_->toString().c_str());
}

TcpServer::~TcpServer() {
    if (main_event_loop_) {
        delete main_event_loop_;
        main_event_loop_ = nullptr;
    }
    // if (io_thread_group_) {
    //     delete io_thread_group_;
    //     io_thread_group_ = nullptr;
    // }
    // if (listen_fd_event_) {
    //     delete listen_fd_event_;
    //     listen_fd_event_ = nullptr;
    // }
}

void TcpServer::start() {
    io_thread_group_->start();
    main_event_loop_->loop();
}

void TcpServer::init() {
    acceptor_ = std::make_shared<TcpAcceptor>(local_addr_);

    main_event_loop_ = EventLoop::GetEventLoopOfCurrentThread();
    io_thread_group_ = new IOThreadGroup(2);

    listen_fd_event_ = new FdEvent(acceptor_->getListenFd());
    listen_fd_event_->listenEvent(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));

    main_event_loop_->addEpollEvent(listen_fd_event_);
}

void TcpServer::onAccept() {
    auto ret = acceptor_->accept();
    int client_fd = ret.first;
    NetAddr::s_ptr peer_addr = ret.second;
    
    client_counts++;

    // TODO: 把 clientfd 添加到任意 IO 线程
    IOThread* io_thread = io_thread_group_->getIOThread();
    TcpConnection::s_ptr connection = std::make_shared<TcpConnection>(io_thread, client_fd, 128, peer_addr);
    connection->setState();

    client_.insert(connection);

    INFOLOG("TcpServer succ get client, fd=%d", client_fd);
}

}