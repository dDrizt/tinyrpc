#include <sys/timerfd.h>
#include <string.h>
#include "tinyrpc/net/timer.h"
#include "tinyrpc/common/log.h"
#include "tinyrpc/common/util.h"

namespace tinyrpc {

Timer::Timer() : FdEvent() {

    fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    DEBUGLOG("timer fd = %d", fd_);

    listenEvent(FdEvent::IN_EVENT, std::bind(&Timer::onTime, this));
}

Timer::~Timer() {
    
}

void Timer::onTime() {
    // uint64_t exp;
    // ssize_t s = read(fd_, &exp, sizeof(uint64_t));
    // if (s != sizeof(uint64_t)) {
    //     ERRORLOG("read() error");
    //     return;
    // }

    char buf[8];
    while(1) {
        if ((read(fd_, buf, 8) == -1) && errno == EAGAIN) {
        break;
        }
    }

    // int64_t now = getCurrentMs();
    // DEBUGLOG("Timer::onTime() now = %ld", now);
    // std::vector<TimerEvent::s_ptr> events;
    // std::vector<std::pair<int64_t, std::function<void()>>> callbacks;
    // {
    //     ScopeMutex<Mutex> lock(mutex_);
    //     auto it = timerEvents_.begin();
    //     while (it != timerEvents_.end()) {
    //         if (it->first <= now) {
    //             if (!it->second->isCancelled()) {
    //                 events.push_back(it->second);
    //                 callbacks.push_back(std::make_pair(it->first, it->second->getCallback()));
    //             }
    //         } else {
    //             break;
    //         }
    //     }

    //     timerEvents_.erase(timerEvents_.begin(), it);
    // }

    // for (auto& event : events) {
    //     if (event->isRepeat()) {
    //         event->resetArriveTime();
    //         addTimerEvent(event);
    //     }
    // }

    // resetTimer();

    // for (auto& callback : callbacks) {
    //     if (callback.second) {
    //         callback.second();
    //     }
    // }

    int64_t now = getCurrentMs();

  std::vector<TimerEvent::s_ptr> tmps;
  std::vector<std::pair<int64_t, std::function<void()>>> tasks;

  ScopeMutex<Mutex> lock(mutex_);
  auto it = timerEvents_.begin();

  for (it = timerEvents_.begin(); it != timerEvents_.end(); ++it) {
    if ((*it).first <= now) {
      if (!(*it).second->isCancelled()) {
        tmps.push_back((*it).second);
        tasks.push_back(std::make_pair((*it).second->getNextTriggerTime(), (*it).second->getCallback()));
      }
    } else {
      break;
    }
  }

  timerEvents_.erase(timerEvents_.begin(), it);
  lock.unlock();


  // 需要把重复的Event 再次添加进去
  for (auto i = tmps.begin(); i != tmps.end(); ++i) {
    if ((*i)->isRepeat()) {
      // 调整 arriveTime
      (*i)->resetArriveTime();
      addTimerEvent(*i);
    }
  }

  resetTimer();

  for (auto i: tasks) {
    if (i.second) {
      i.second();
    }
  }
}

/*
void Timer::resetTimer() {
    if (timerEvents_.empty()) {
        return;
    }

    auto it = timerEvents_.begin();
    int64_t nextTriggerTime = it->first;
    int64_t now = getCurrentMs();
    int64_t interval = nextTriggerTime - now;
    if (nextTriggerTime <= now) {
        nextTriggerTime = now + 1;
    } else {
        nextTriggerTime = it->first;
    }
    DEBUGLOG("Timer::resetTimer() nextTriggerTime = %ld, now = %ld, interval = %ld", nextTriggerTime, now, interval);
    struct itimerspec newValue;
    newValue.it_value.tv_sec = interval / 1000;
    newValue.it_value.tv_nsec = (interval % 1000) * 1000000;
    newValue.it_interval.tv_sec = 0;
    newValue.it_interval.tv_nsec = 0;
    int ret = timerfd_settime(fd_, 0, &newValue, nullptr);
    if (ret < 0) {
        ERRORLOG("timerfd_settime() error");
    }
    DEBUGLOG("timer rest to &lld", now + interval); 
}
*/

void Timer::resetTimer() {
    ScopeMutex<Mutex> lock(mutex_);
    auto tmp = timerEvents_;
    lock.unlock();

    if (tmp.size() == 0) {
        return;
    }

    int64_t now = getCurrentMs();

    auto it = tmp.begin();
    int64_t inteval = 0;
    if (it->second->getNextTriggerTime() > now) {
        inteval = it->second->getNextTriggerTime() - now;
    } else {
        inteval = 100;
    }

    timespec ts;
    memset(&ts, 0, sizeof(ts));
    ts.tv_sec = inteval / 1000;
    ts.tv_nsec = (inteval % 1000) * 1000000;

    itimerspec value;
    memset(&value, 0, sizeof(value));
    value.it_value = ts;

    int rt = timerfd_settime(fd_, 0, &value, NULL);
    if (rt != 0) {
        ERRORLOG("timerfd_settime error, errno=%d, error=%s", errno, strerror(errno));
    }
    // DEBUGLOG("timer reset to %lld", now + inteval);
}

void Timer::addTimerEvent(TimerEvent::s_ptr event) {
    // bool is_reset = false;
    // {
    //     ScopeMutex<Mutex> lock(mutex_);
    //     auto it = timerEvents_.find(event->getNextTriggerTime());
    //     if (it != timerEvents_.end()) {
    //         if (event->getNextTriggerTime() < it->first) {
    //             is_reset = true;
    //         }
    //         // timerEvents_.erase(it);
    //     }
    //     timerEvents_.insert(std::make_pair(event->getNextTriggerTime(), event));
    // }

    // if (is_reset) {
    //     resetTimer();
    // }

    bool is_reset = false;

    {
        ScopeMutex<Mutex> lock(mutex_);
        if (timerEvents_.empty()) {
            is_reset = true;
        } else {
            auto it = timerEvents_.begin();
            if (event->getNextTriggerTime() < it->second->getNextTriggerTime()) {
                is_reset = true;
            }
        }
        timerEvents_.emplace(event->getNextTriggerTime(), event);
    }

    if (is_reset)
        resetTimer();
}

void Timer::deleteTimerEvent(TimerEvent::s_ptr event) {
    event->setCancelled(true);

    ScopeMutex<Mutex> lock(mutex_);
    
    auto begin = timerEvents_.lower_bound(event->getNextTriggerTime());
    auto end = timerEvents_.upper_bound(event->getNextTriggerTime());

    for (auto it = begin; it != end; ++it) {
        if (it->second == event) {
            timerEvents_.erase(it);
            break;
        }
    }
    lock.unlock();
    DEBUGLOG("Timer::deleteTimerEvent() timerEvents_.size() = %ld", timerEvents_.size());


}


}  // namespace tinyrpc
