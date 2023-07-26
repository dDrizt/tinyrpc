#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include "tinyrpc/common/log.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_acceptor.h"

namespace tinyrpc {

TcpAcceptor::TcpAcceptor(NetAddr::s_ptr local_addr) : local_addr_(local_addr) {
    if (!local_addr->checkValid()) {
        ERRORLOG("Invalid local addr %s", local_addr->toString().c_str());
        exit(0);
    }

    family_ = local_addr->getFamily();

    listenfd_ = socket(family_, SOCK_STREAM, 0);

    if (listenfd_ < 0) {
        ERRORLOG("Invalid listenfd %d", listenfd_);
        exit(0);
    }

    int val = 1;
    if (setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0) {
        ERRORLOG("setsockopt REUSEADDR error, errno=%d, error=%s", errno, strerror(errno));
    }

    socklen_t len = local_addr->getSockLen();
    if (bind(listenfd_, local_addr_->getSockAddr(), len) != 0) {
        ERRORLOG("bind error, errno=%d, error=%s", errno, strerror(errno));
        exit(0);
    }

    if (listen(listenfd_, 1000) != 0) {
        ERRORLOG("listen error, errno=%d, error=%s", errno, strerror(errno));
        exit(0);
    }
}

TcpAcceptor::~TcpAcceptor() {

}

int TcpAcceptor::getListenFd() {
    return listenfd_;
}

std::pair<int, NetAddr::s_ptr> TcpAcceptor::accept() {
    if (family_ == AF_INET) {
        sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_addr_len = sizeof(client_addr);

        int client_fd = ::accept(listenfd_, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
        if (client_fd < 0)
            ERRORLOG("accept error, errno=%d, error=%s", errno, strerror(errno));

        IPNetAddr::s_ptr peer_addr = std::make_shared<IPNetAddr>(client_addr);
        INFOLOG("A client have accepted succ, peer addr [%s]", peer_addr->toString().c_str());

        return std::make_pair(client_fd, peer_addr);
    } else {
        // ...
        return std::make_pair(-1, nullptr);
    }
}

}