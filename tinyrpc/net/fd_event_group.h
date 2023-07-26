#pragma once

#include <vector>
#include "tinyrpc/net/fdevent.h"
#include "tinyrpc/common/mutex.h"

namespace tinyrpc {

class FdEventGroup {

public:
    FdEventGroup(int size);
    ~FdEventGroup();

    FdEvent* getFdEvent(int fd);

public:
    static FdEventGroup* GetFdEventGroup();

private:
    int size_ {0};
    std::vector<FdEvent*> fd_group_;
    Mutex mutex_;

};

}