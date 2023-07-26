#include "tinyrpc/common/mutex.h"
#include "tinyrpc/common/log.h"
#include "tinyrpc/net/fd_event_group.h"

namespace tinyrpc {

static FdEventGroup* g_fd_event_group = nullptr;

FdEventGroup* FdEventGroup::GetFdEventGroup() {
    if (g_fd_event_group != nullptr)
        return g_fd_event_group;
    
    g_fd_event_group = new FdEventGroup(128);

    return g_fd_event_group;
}

FdEventGroup::FdEventGroup(int size) : size_(size) {
    for (int i = 0; i < size_; ++i)
        fd_group_.push_back(new FdEvent(i));
}

FdEventGroup::~FdEventGroup() {
    for (int i = 0; i < size_; ++i) {
        if (fd_group_[i] != NULL) {
            delete fd_group_[i];
            fd_group_[i] = nullptr;
        }
    }
}

FdEvent* FdEventGroup::getFdEvent(int fd) {
    ScopeMutex<Mutex> lock(mutex_);
    if (static_cast<size_t>(fd) < fd_group_.size())
        return fd_group_[fd];
    
    int new_size = static_cast<int>(fd * 1.5);
    for (int i = fd_group_.size(); i < new_size; ++i)
        fd_group_.push_back(new FdEvent(i));
    
    return fd_group_[fd];
}

}