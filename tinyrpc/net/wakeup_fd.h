#pragma once 

#include "tinyrpc/common/log.h"
#include "tinyrpc/net/fdevent.h"

namespace tinyrpc {

class WakeUpFdEvent : public FdEvent {
public:
    WakeUpFdEvent(int fd) : FdEvent(fd) {}
    ~WakeUpFdEvent() = default;

    void inline wakeup() {
        uint64_t one = 1;
        ssize_t n = write(fd_, &one, sizeof(one));
        if (n != sizeof(one)) {
            ERRORLOG("WakeUpFdEvent::wakeup() error");
        }
        DEBUGLOG("WakeUpFdEvent::wakeup() success");
    };

};

} // namespacetiny 
