#pragma once

#include <map>
#include "tinyrpc/common/mutex.h"
#include "tinyrpc/net/fdevent.h"
#include "tinyrpc/net/timer_event.h"

namespace tinyrpc {

class Timer : public FdEvent {
public:
    Timer();

    ~Timer();

    void addTimerEvent(TimerEvent::s_ptr timerEvent);

    void deleteTimerEvent(TimerEvent::s_ptr timerEvent);

    void onTime();  // 当发送了 IO 事件后，eventloop 会执行这个回调函数

private:
    void resetTimer();
    
private:
    std::multimap<int64_t, TimerEvent::s_ptr> timerEvents_;

    Mutex mutex_;
};

} // namespace tinyrpc