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
#include "tinyrpc/net/timer_event.h"
#include "tinyrpc/net/io_thread.h"
#include "tinyrpc/net/io_thread_group.h"

// void test_io_thread() {

//     int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (listen_fd < 0) {
//         ERRORLOG("create socket error, errno: %d, error info: %s", errno, strerror(errno));
//         exit(0);
//     }

//     sockaddr_in addr;
//     memset(&addr, 0, sizeof(addr));

//     addr.sin_port = htons(12345);
//     addr.sin_family = AF_INET;
//     addr.sin_addr.s_addr = inet_addr("127.0.0.1");

//     int ret = bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
//     if (ret < 0) {
//         ERRORLOG("bind error, errno: %d, error info: %s", errno, strerror(errno));
//         exit(0);
//     }

//     ret = listen(listen_fd, 10);
//     if (ret < 0) {
//         ERRORLOG("listen error, errno: %d, error info: %s", errno, strerror(errno));
//         exit(0);
//     }

//     tinyrpc::FdEvent* fd_event = new tinyrpc::FdEvent(listen_fd);
//     fd_event->listenEvent(tinyrpc::FdEvent::IN_EVENT, [listen_fd](){
//         sockaddr_in client_addr;
//         socklen_t client_addr_len = sizeof(client_addr);
//         memset(&client_addr, 0, sizeof(client_addr));
//         int client_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);

//         DEBUGLOG("success accept client, client [%s:%d]", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
//     });

//     int i = 0;
//     tinyrpc::TimerEvent::s_ptr timer_event = std::make_shared<tinyrpc::TimerEvent>(
//         1000, true, [&i]() {
//             INFOLOG("trigger timer event %d", i++);
//         }
//     );

//     // tinyrpc::IOThread io_thread;

//     // io_thread.getLoop()->addEpollEvent(fd_event);
//     // io_thread.getLoop()->addTimerEvent(timer_event);
//     // io_thread.start();

//     // io_thread.join();

//     tinyrpc::IOThreadGroup io_thread_group(2);
//     tinyrpc::IOThread* io_thread = io_thread_group.getIOThread();
//     io_thread->getLoop()->addEpollEvent(fd_event);
//     io_thread->getLoop()->addTimerEvent(timer_event);

//     tinyrpc::IOThread* io_thread2 = io_thread_group.getIOThread();
//     io_thread2->getLoop()->addTimerEvent(timer_event);

//     io_thread_group.start();
//     io_thread_group.join();
// }

// int main() {

//     tinyrpc::Config::SetGlobalConfigPath("../conf/tinyrpc.xml");

//     tinyrpc::Logger::InitGlobalLogger();

//     test_io_thread();

//     // tinyrpc::EventLoop* loop = new tinyrpc::EventLoop();

//     return 0;
// }

int main() {

  tinyrpc::Config::SetGlobalConfigPath("../conf/tinyrpc.xml");

  tinyrpc::Logger::InitGlobalLogger();

  tinyrpc::EventLoop* eventloop = new tinyrpc::EventLoop();

  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd == -1) {
    ERRORLOG("listenfd = -1");
    exit(0);
  }

  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_port = htons(12310);
  addr.sin_family = AF_INET;
  inet_aton("127.0.0.1", &addr.sin_addr);

  int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
  if (rt != 0) {
    ERRORLOG("bind error");
    exit(1);
  }

  rt = listen(listenfd, 100);
  if (rt != 0) {
    ERRORLOG("listen error");
    exit(1);
  }

  tinyrpc::FdEvent event(listenfd);
  event.listenEvent(tinyrpc::FdEvent::IN_EVENT, [listenfd](){
    sockaddr_in peer_addr;
    socklen_t addr_len = sizeof(peer_addr);
    memset(&peer_addr, 0, sizeof(peer_addr));
    int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len);

    DEBUGLOG("success get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

  });
  eventloop->addEpollEvent(&event);

  eventloop->loop();

  return 0;
}