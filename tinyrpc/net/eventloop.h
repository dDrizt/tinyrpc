#pragma once

#include <pthread.h>
#include <string.h>
#include <set>
#include <queue>
#include <functional>

#include "tinyrpc/common/mutex.h"
#include "tinyrpc/net/fdevent.h"
#include "tinyrpc/net/wakeup_fd.h"
#include "tinyrpc/common/util.h"
#include "tinyrpc/net/timer.h"


namespace tinyrpc {

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    void loop();

    void wakeup();

    void stop();

    void addEpollEvent(FdEvent* fd_event);

    void delEpollEvent(FdEvent* fd_event);

    void addTask(std::function<void()> cb, bool is_immediate = false);

    void addTimerEvent(TimerEvent::s_ptr timer_event);

    EventLoop* getEventLoopOfCurrentThread();

    bool isLooping() {
        return is_looping;
    }

    bool isInLoopThread() const {
        return threadId_ == getThreadId();
    }

private:
    void dealWakeup();

    void initWakeUpFdEvent();

    void initTimer();

private:

    template <typename MapType, typename EventType>
    void addToEpoll(int epollFd, MapType &listenFds, const EventType &fd_event) {
        auto it = listenFds_.find(fd_event->getFd());
        int op = EPOLL_CTL_ADD;
        if (it != listenFds_.end()) {
            op = EPOLL_CTL_MOD;
        }
        epoll_event event = fd_event->getEvent();
        INFOLOG("epoll_event.events = %d", (int)event.events);
        int ret = epoll_ctl(epollFd_, op, fd_event->getFd(), &event);
        if (ret < 0) {
            ERRORLOG("epoll_ctl error when add fd, errno: %d, error info: %s", errno, strerror(errno));
            return;
        }
        listenFds_.insert(fd_event->getFd());
        DEBUGLOG("add fd %d to epoll", fd_event->getFd());
    }

    template <typename MapType, typename EventType>
    void deleteFromEpoll(int epollFd, MapType &listenFds, const EventType &fd_event) {
        auto it = listenFds_.find(fd_event->getFd());
        if (it == listenFds_.end()) {
            return;
        }
        int ret = epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd_event->getFd(), nullptr);
        if (ret < 0) {
            ERRORLOG("epoll_ctl error when delete fd, errno: %d, error info: %s", errno, strerror(errno));
            return;
        }
        listenFds_.erase(it);
        DEBUGLOG("delete fd %d from epoll", fd_event->getFd());
    }

private:
    pid_t threadId_ {0};  

    int epollFd_ {0};

    int wakeupFd_ {0};

    WakeUpFdEvent* wakeupFdEvent_ {nullptr};

    bool is_stop_ {false};

    std::set<int> listenFds_;

    std::queue<std::function<void()>> pendingFunctors_;

    Timer* timer_ {nullptr};

    Mutex mutex_;

    bool is_looping {false};
};

}   // namespace tinyrpc