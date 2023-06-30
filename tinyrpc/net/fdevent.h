#pragma once

#include <sys/epoll.h>
#include <functional>

namespace tinyrpc {
class FdEvent {
public:
    enum TriggerType {
        IN_EVENT = EPOLLIN,
        OUT_EVENT = EPOLLOUT,
        ERROR_EVENT = EPOLLERR, 
    };

    FdEvent();
    FdEvent(int fd);

    ~FdEvent();

    void setNonBlock();

    std::function<void()> handleEvent(TriggerType event_type);

    void listenEvent(TriggerType event_type, std::function<void()> callback, std::function<void()> error_callback = nullptr);

    void cancle(TriggerType event_type);

    int getFd() const {
        return fd_;
    }

    epoll_event& getEvent() {
        return event_;
    }    

protected:
    int fd_ {-1};

    epoll_event event_;

    std::function<void()> readCallback_;

    std::function<void()> writeCallback_;

    std::function<void()> errorCallback_;

};

}   // namespace tinyrpc