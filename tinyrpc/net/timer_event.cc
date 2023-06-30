#include "tinyrpc/net/timer_event.h"
#include "tinyrpc/common/log.h"
#include "tinyrpc/common/util.h"

namespace tinyrpc {

TimerEvent::TimerEvent(int interval, bool repeat, std::function<void()> cb) : interval_(interval), repeat_(repeat), task_(cb) {
    resetArriveTime();    
}

void TimerEvent::resetArriveTime() {
    nextTriggerTime_ = getCurrentMs() + interval_;
}


}   // namespace tinyrpc