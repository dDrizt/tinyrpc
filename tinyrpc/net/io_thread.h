#pragma once

#include <pthread.h>
#include <semaphore.h>
#include "tinyrpc/net/eventloop.h"

namespace tinyrpc {

class IOThread {
public:
    IOThread();
    ~IOThread();

public:
    void start();

    void join();

    EventLoop* getLoop() {
        return loop_;
    }

public:
    static void* thread_func(void* arg);

private:
    pid_t thread_id_ {-1};
    pthread_t thread_ {0};

    EventLoop* loop_ {nullptr}; // 当前线程的EventLoop对象

    sem_t init_sem_; // 信号量，用于线程同步

    sem_t start_sem_;
};

}   // namespace tinyrpc