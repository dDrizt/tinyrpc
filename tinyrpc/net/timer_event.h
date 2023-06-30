#pragma once

#include <functional>
#include <memory>

namespace tinyrpc {

class TimerEvent {

public:
    using s_ptr = std::shared_ptr<TimerEvent>;

    // TimerEvent(int interval, std::function<void()> callback, bool repeat = false);

    TimerEvent(int interval, bool repeat, std::function<void()> callback);


    void run() {
        task_();
    }

    int64_t getNextTriggerTime() const {
        return nextTriggerTime_;
    }
    void setNextTriggerTime(int64_t nextTriggerTime) {
        nextTriggerTime_ = nextTriggerTime;
    }

    void setCancelled(bool cancelled) {
        cancelled_ = cancelled;
    }

    bool isCancelled() const {
        return cancelled_;
    }

    bool isRepeat() const {
        return repeat_;
    }

    std::function<void()> getCallback() const {
        return task_;
    }

    void resetArriveTime();

private:
    int64_t interval_ {0};
    int64_t nextTriggerTime_ {0};

    bool repeat_ {false};
    bool cancelled_ {false};

    std::function<void()> task_;
    
};

}  // namespace tinyrpc