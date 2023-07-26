#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/net/fd_event_group.h"
#include "tinyrpc/common/log.h"

namespace tinyrpc {

TcpConnection::TcpConnection(IOThread* io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr)
    : io_thread_(io_thread), peer_addr_(peer_addr), state_(TcpState::NotConnected), fd_(fd)
{
    in_buffer_ = std::make_shared<TcpBuffer>(buffer_size);
    out_buffer_ = std::make_shared<TcpBuffer>(buffer_size);   

    fd_event_ = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    fd_event_->setNonBlock();
    fd_event_->listenEvent(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));

    io_thread->getLoop()->addEpollEvent(fd_event_);

}

TcpConnection::~TcpConnection() {
    DEBUGLOG("~TcpConnection");
}

void TcpConnection::onRead() {

    // 1. 从 socket 缓冲区，调用系统的 read 函数读取字节到 in_buffer
    if (state_ != TcpState::Connected) {
        ERRORLOG("onRead() error, client has already disconnected, addr[%s], clientFd[%d]", peer_addr_->toString().c_str(), fd_);
        return;
    }

    bool is_read_all = false;
    bool is_closed = false;

    while (!is_read_all) {
        if (in_buffer_->writeAble() == 0)
            in_buffer_->resizeBuffer(2 * in_buffer_->buffer_.size());
    
        int read_count = in_buffer_->writeAble();
        int write_index = in_buffer_->writeIndex();

        int ret = read(fd_, &(in_buffer_->buffer_[write_index]), read_count);
        DEBUGLOG("Success read %d bytes from addr[%s], clientFd[%d]", ret, peer_addr_->toString().c_str(), fd_);
        if (ret > 0) {
            in_buffer_->moveWriteIndex(ret);
            if (ret == read_count)
                continue;
            else if (ret < read_count) {
                is_read_all = true;
                break;
            }
        } else if (ret == 0) {
            is_closed = true;
            break;
        } else if (ret == -1 && errno == EAGAIN) {
            is_read_all = true;
            break;
        }
    }

    if (is_closed) {
        // TODO: 处理关闭连接
        INFOLOG("peer closed, peer addr [%d], clientFd [%d]", peer_addr_->toString().c_str(), fd_);
        clear();
        return;
    }

    if (!is_read_all)
        ERRORLOG("Not read all data!");

    // TODO: 简单的 echo，后面补充 RPC 协议解析
    excute();

}

void TcpConnection::excute() {
    // 将 RPC 请求执行业务逻辑， 获取 RPC 响应，再把 RPC 响应发送回去
    // int size = in_buffer_->readAble();
    // std::vector<char> tmp(size);
    std::vector<char> tmp;
    int size = in_buffer_->readAble();
    tmp.resize(size);
    in_buffer_->readFromBuffer(tmp, size);

    std::string msg;
    for (size_t i = 0; i < tmp.size(); ++i)
        msg += tmp[i];

    INFOLOG("Success get request [%s] from client [%s]", msg.c_str(), peer_addr_->toString().c_str());

    out_buffer_->writeToBuffer(msg.c_str(), msg.length());

    fd_event_->listenEvent(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
    
    io_thread_->getLoop()->addEpollEvent(fd_event_);

}

void TcpConnection::onWrite() {
    // 将当前 out_buffer 里的数据全部发送给 client
    if (state_ != TcpState::Connected) {
        ERRORLOG("onWrite() error, client has already disconnected, addr[%s], clientFd[%d]", peer_addr_->toString().c_str(), fd_);
        return;
    }

    bool is_write_all = false;
    while (true) {
        if (out_buffer_->readAble() == 0) {
            DEBUGLOG("No data need to send to client [%s]", peer_addr_->toString().c_str());
            is_write_all = true;
            break;
        }

        int write_size = out_buffer_->readAble();
        int read_index = out_buffer_->readIndex();

        int ret = write(fd_, &(out_buffer_->buffer_[read_index]), write_size);

        if (ret >= write_size) {
            DEBUGLOG("No data need to send to client [%s]", peer_addr_->toString().c_str());
            is_write_all = true;
            break;
        }
        if (ret == -1 && errno == EAGAIN) {
            // 发送缓冲区已满，不能再发送。
            // 这种情况，需要等待下次 fd 可写时，再次发送数据
            ERRORLOG("Write data error, error==EAGAIN and ret == -1");
            break;  
        }
    }
    
    if (is_write_all) {
        fd_event_->cancle(FdEvent::OUT_EVENT);
        io_thread_->getLoop()->addEpollEvent(fd_event_);
    }
}

void TcpConnection::setState(const TcpConnection::TcpState state) {
    state_ = state;
}

TcpConnection::TcpState TcpConnection::getState() {
    return state_;
}

void TcpConnection::clear() {
    // 进行关闭连接后的清理动作
    if (state_ == TcpState::Closed)
        return;
    
    fd_event_->cancle(FdEvent::IN_EVENT);
    fd_event_->cancle(FdEvent::OUT_EVENT);

    io_thread_->getLoop()->delEpollEvent(fd_event_);

    state_ = TcpState::Closed;
    
}

void TcpConnection::shutdown() {
    if (state_ == TcpState::Closed || state_ == TcpState::NotConnected)
        return;
    
    state_ = TcpState::HalfClosing;

    // 调用 shutdown 关闭读写，服务器不会再对这个 fd 进行读写操作
    // 发送 FIN 报文，触发了四次挥手的第一个阶段
    // 当 fd 发生了可读事件，但是可读的数据为 0，即对端发送了 FIN
    ::shutdown(fd_, SHUT_RDWR);
}

}