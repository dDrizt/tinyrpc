#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/eventfd.h>
#include "tinyrpc/common/log.h"
#include "tinyrpc/common/util.h"
#include "tinyrpc/net/eventloop.h"

namespace tinyrpc {

static thread_local EventLoop* t_loopInThisThread = nullptr;
static int g_epoll_max_events = 10;
static int g_epoll_max_timeout = 10000;

// static int createEventFd() {
//     int eventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
//     if (eventFd < 0) {
//         ERRORLOG("eventfd error, errno: %d, error info: %s", errno, strerror(errno));
//         exit(0);
//     }
//     return eventFd;
// } 

EventLoop::EventLoop() {
    if (t_loopInThisThread != nullptr) {
        ERRORLOG("Failed to create EventLoop, Another EventLoop exists in this thread");
        exit(0);
    }

    threadId_ = getThreadId();

    epollFd_ = epoll_create(10);
    if (epollFd_ < 0) {
        ERRORLOG("epoll_create1 error, errno: %d, error info: %s", errno, strerror(errno));
        exit(0);
    }

    initWakeUpFdEvent();
    initTimer();

    INFOLOG("SUCESS, EventLoop created in thread %d", threadId_);
    t_loopInThisThread = this;
}

EventLoop::~EventLoop() {
    close(epollFd_);
    if (wakeupFdEvent_ != nullptr) {
        delete wakeupFdEvent_;
        wakeupFdEvent_ = nullptr;
    }

    if (timer_ != nullptr) {
        delete timer_;
        timer_ = nullptr;
    }
}

void EventLoop::loop() {
    is_looping = true;
    while (!is_stop_) {
        ScopeMutex<Mutex> lock(mutex_);
        // std::queue<std::function<void()>> tmpFunctors = std::move(pendingFunctors_);
        
        std::queue<std::function<void()>> tmpFunctors;
        pendingFunctors_.swap(tmpFunctors);

        lock.unlock();

        while (!tmpFunctors.empty()) {
            std::function<void()> cb = tmpFunctors.front();
            tmpFunctors.pop();
            if (cb)
                cb();
        }

        int timeout = g_epoll_max_timeout;
        epoll_event events[g_epoll_max_events];
        DEBUGLOG("now begin to epoll_wait");
        int numEvents = epoll_wait(epollFd_, events, g_epoll_max_events, timeout);
        DEBUGLOG("now end epoll_wait, rt = %d", numEvents);

        if (numEvents < 0) {
            ERRORLOG("epoll_wait error, errno: %d, error info: %s", errno, strerror(errno));
        }
        else {
            for (int i = 0; i < numEvents; ++i) {
                epoll_event trigger_event = events[i];
                FdEvent* fdEvent = static_cast<FdEvent*>(trigger_event.data.ptr);
                if (fdEvent == nullptr) {
                    ERRORLOG("epoll_wait get nullptr");
                    continue;
                }

                if (trigger_event.events & EPOLLIN) {
                    DEBUGLOG("fd %d trigger EPOLLIN event", fdEvent->getFd());
                    addTask(fdEvent->handleEvent(FdEvent::IN_EVENT));

                }

                if (trigger_event.events & EPOLLOUT)
                    addTask(fdEvent->handleEvent(FdEvent::OUT_EVENT));

                if (trigger_event.events & EPOLLERR) {
                    DEBUGLOG("fd %d trigger EPOLLERROR", fdEvent->getFd());
                    delEpollEvent(fdEvent);
                    if (fdEvent->handleEvent(FdEvent::ERROR_EVENT) != nullptr) {
                        DEBUGLOG("fd %d add error callback", fdEvent->getFd());
                        addTask(fdEvent->handleEvent(FdEvent::OUT_EVENT));
                    }
                }
            }
        }
    }
}

void EventLoop::wakeup() {
    INFOLOG("wakeup");
    wakeupFdEvent_->wakeup();
}

void EventLoop::stop() {
    is_stop_ = true;
    wakeup();
}

void EventLoop::addEpollEvent(FdEvent* fd_event) {
    if (isInLoopThread()) {
        addToEpoll<decltype(listenFds_), FdEvent*>(epollFd_, listenFds_, fd_event);
    } else {
        auto cb = [this, fd_event]() {
            addToEpoll<decltype(listenFds_), FdEvent*>(epollFd_, listenFds_, fd_event);
        };
        addTask(cb, true);
    }
}

void EventLoop::delEpollEvent(FdEvent* fd_event) {
    if (isInLoopThread()) {
        deleteFromEpoll<decltype(listenFds_), FdEvent*>(epollFd_, listenFds_, fd_event);
    } else {
        auto cb = [this, fd_event]() {
            deleteFromEpoll<decltype(listenFds_), FdEvent*>(epollFd_, listenFds_, fd_event);
        };
        addTask(cb, true);
    }
}

void EventLoop::addTask(std::function<void()> cb, bool is_immediate) {
    if (is_immediate) {
        wakeup();
    } else {
        ScopeMutex<Mutex> lock(mutex_);
        pendingFunctors_.push(cb);
    }
}

void EventLoop::initWakeUpFdEvent() {
    wakeupFd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeupFd_ < 0) {
        ERRORLOG("failed to create event loop, errno: %d, error info: %s", errno, strerror(errno));
        exit(0);
    }
    INFOLOG("wakeupFd_ = %d", wakeupFd_);

    wakeupFdEvent_ = new WakeUpFdEvent(wakeupFd_);
    wakeupFdEvent_->listenEvent(FdEvent::IN_EVENT, [this]() {
        // uint64_t one = 1;
        // ssize_t n = read(wakeupFd_, &one, sizeof(one));
        // while (n != -1 && errno != EAGAIN) {}
        // DEBUGLOG("EventLoop::wakeupFdEvent_ read %ld bytes", n)

        char buf[8];
        while(read(wakeupFd_, buf, 8) != -1 && errno != EAGAIN) {
        }
        DEBUGLOG("read full bytes from wakeup fd[%d]", wakeupFd_);
    });

    addEpollEvent(wakeupFdEvent_);
}

void EventLoop::initTimer() {
    timer_ = new Timer();
    addEpollEvent(timer_);
}

void EventLoop::addTimerEvent(TimerEvent::s_ptr timer_event) {
    timer_->addTimerEvent(timer_event);
}

EventLoop* EventLoop::GetEventLoopOfCurrentThread() {
    if (t_loopInThisThread) {
        return t_loopInThisThread;
    }
    t_loopInThisThread = new EventLoop();
    return t_loopInThisThread;
}

}  // namespace tinyrpc

