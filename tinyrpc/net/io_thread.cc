#include <pthread.h>
#include <assert.h>
#include "tinyrpc/net/io_thread.h"
#include "tinyrpc/common/log.h"
#include "tinyrpc/common/util.h"

namespace tinyrpc {

IOThread::IOThread() {
    int ret = sem_init(&init_sem_, 0, 0);
    assert(ret == 0);

    ret = sem_init(&start_sem_, 0, 0);
    assert(ret == 0);

    pthread_create(&thread_, nullptr, &IOThread::thread_func, this);

    sem_wait(&init_sem_);
    DEBUGLOG("IOThread::IOThread() success, IOThread id = %d", thread_id_);
}

IOThread::~IOThread() {
    loop_->stop();

    sem_destroy(&init_sem_);
    sem_destroy(&start_sem_);

    pthread_join(thread_, nullptr);

    if (loop_ != nullptr) {
        delete loop_;
        loop_ = nullptr;
    }
}

void* IOThread::thread_func(void* arg) {
    IOThread* io_thread = static_cast<IOThread*> (arg);

    io_thread->loop_ = new EventLoop();
    io_thread->thread_id_ = getThreadId();

    sem_post(&io_thread->init_sem_);
    DEBUGLOG("IOThread::thread_func() success, IOThread id = %d", io_thread->thread_id_);

    // 让 IO 线程等待，直到我们主动启动
    sem_wait(&io_thread->start_sem_);
    DEBUGLOG("IOThread::thread_func() start, IOThread id = %d", io_thread->thread_id_);
    io_thread->loop_->loop();

    DEBUGLOG("IOThread::thread_func() end, IOThread id = %d", io_thread->thread_id_);

    return nullptr;

}

void IOThread::start() {
    DEBUGLOG("IOThread::start() start, IOThread id = %d", thread_id_);
    sem_post(&start_sem_);
}

void IOThread::join() {
    pthread_join(thread_, nullptr);
}

}   // namespace tinyrpc