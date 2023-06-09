#pragma once


namespace tinyrpc {

template <class T>
class ScopeMutex {
public:
    ScopeMutex(T& mutex) : mutex_(mutex) {
        mutex_.lock();
        is_locked_ = true;
    }

    ~ScopeMutex() {
        if (is_locked_) {
            mutex_.unlock();
        }

        is_locked_ = false;
    }

    void lock() {
        if (!is_locked_) {
            mutex_.lock();
            is_locked_ = true;
        }
    }

    void unlock() {
        if (is_locked_) {
            mutex_.unlock();
            is_locked_ = false;
        }
    }
private:
    T& mutex_;

    bool is_locked_ { false };
};

class Mutex {
public:
    Mutex() {
        pthread_mutex_init(&mutex_, nullptr);
    }

    ~Mutex() {
        pthread_mutex_destroy(&mutex_);
    }

    void lock() {
        pthread_mutex_lock(&mutex_);
    }

    void unlock() {
        pthread_mutex_unlock(&mutex_);
    }

private:
    pthread_mutex_t mutex_;

};

}      // namespace tinyrpc

