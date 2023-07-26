#include "tinyrpc/common/log.h"
#include "tinyrpc/net/io_thread_group.h"

namespace tinyrpc {

IOThreadGroup::IOThreadGroup(int size) : size_(size) {
    io_thread_groups_.resize(size);
    for (size_t i = 0; i < size; ++i)
        io_thread_groups_[i] = new IOThread();
}

IOThreadGroup::~IOThreadGroup() {

}

void IOThreadGroup::start() {
    // for (auto& io_thread : io_thread_groups_)
    //     io_thread->start();
    for (size_t i = 0; i < io_thread_groups_.size(); ++i)
        io_thread_groups_[i]->start();
}

void IOThreadGroup::join() {
    // for (auto& io_thread : io_thread_groups_)
    //     io_thread->join();
    for (size_t i = 0; i < io_thread_groups_.size(); ++i)
        io_thread_groups_[i]->join();
}

IOThread* IOThreadGroup::getIOThread() {
    if (index_ == static_cast<int> (io_thread_groups_.size()) || index_ == -1)
        index_ = 0;
    
    return io_thread_groups_[index_++];
}

}   // namespace tinyrpc