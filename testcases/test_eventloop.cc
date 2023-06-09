#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <memory>
#include "tinyrpc/common/log.h"
#include "tinyrpc/common/config.h"
#include "tinyrpc/net/fdevent.h"
#include "tinyrpc/net/eventloop.h"

int main() {

    tinyrpc::Config::SetGlobalConfigPath("/mnt/Disk__/Dev/tinyrpc/conf/tinyrpc.xml");

    tinyrpc::Logger::InitGlobalLogger();

    tinyrpc::EventLoop* loop = new tinyrpc::EventLoop();

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        ERRORLOG("create socket error, errno: %d, error info: %s", errno, strerror(errno));
        return -1;
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_port = htons(12345);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (ret < 0) {
        ERRORLOG("bind error, errno: %d, error info: %s", errno, strerror(errno));
        return -1;
    }

    ret = listen(listen_fd, 10);
    if (ret < 0) {
        ERRORLOG("listen error, errno: %d, error info: %s", errno, strerror(errno));
        return -1;
    }

    tinyrpc::FdEvent* fd_event = new tinyrpc::FdEvent(listen_fd);
    fd_event->listenEvent(tinyrpc::FdEvent::IN_EVENT, [listen_fd](){
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        memset(&client_addr, 0, sizeof(client_addr));
        int client_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);

        DEBUGLOG("success accept client, client [%s:%d]", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    });

    loop->addEpollEvent(fd_event);

    loop->loop();

    return 0;
}