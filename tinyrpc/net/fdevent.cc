#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "tinyrpc/net/fdevent.h"
#include "tinyrpc/common/log.h"

namespace tinyrpc {

FdEvent::FdEvent(int fd) : fd_(fd) {
    memset(&event_, 0, sizeof(event_));
}

FdEvent::FdEvent() {
    memset(&event_, 0, sizeof(event_));
}

FdEvent::~FdEvent() {
    close(fd_);
}

std::function<void()> FdEvent::handleEvent(TriggerType event_type) {
    if (event_type == TriggerType::IN_EVENT) {
        return readCallback_;
    } else if (event_type == TriggerType::OUT_EVENT) {
        return writeCallback_;
    } else if (event_type == TriggerType::ERROR_EVENT) {
        return errorCallback_;
    } else {
        ERRORLOG("Unknown event type");
        return nullptr;
    }
}

void FdEvent::listenEvent(TriggerType event_type, std::function<void()> callback, std::function<void()> error_callback) {
    if (event_type == TriggerType::IN_EVENT) {
        event_.events |= EPOLLIN;
        readCallback_ = callback;
    } else {
        event_.events |= EPOLLOUT;
        writeCallback_ = callback;
    }

    if (error_callback != nullptr) {
        event_.events |= EPOLLERR;
        errorCallback_ = error_callback;
    } else {
        errorCallback_ = nullptr;
    }

    event_.data.ptr = this;
}

void FdEvent::cancle(TriggerType event_type) {
    if (event_type == TriggerType::IN_EVENT) {
        event_.events &= ~EPOLLIN;
        readCallback_ = nullptr;
    } else if (event_type == TriggerType::OUT_EVENT) {
        event_.events &= ~EPOLLOUT;
        writeCallback_ = nullptr;
    } else if (event_type == TriggerType::ERROR_EVENT) {
        event_.events &= ~EPOLLERR;
        errorCallback_ = nullptr;
    } else {
        ERRORLOG("Unknown event type");
    }
}

void FdEvent::setNonBlock() {
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags < 0) {
        ERRORLOG("fcntl get flags error");
        return;
    }

    flags |= O_NONBLOCK;
    if (fcntl(fd_, F_SETFL, flags) < 0) {
        ERRORLOG("fcntl set flags error");
        return;
    }

}



}