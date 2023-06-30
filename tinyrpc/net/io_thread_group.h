#pragma once

#include <vector>
#include "tinyrpc/common/log.h"
#include "tinyrpc/net/io_thread.h"

namespace tinyrpc {

class IOThreadGroup {

public:
    IOThreadGroup(int size);

    ~IOThreadGroup();

    void start();

    void join();

    IOThread* getIOThread();


private:
    int size_ {0};
    int index_ {0};

    std::vector<IOThread*> io_thread_groups_;

};

}   // namespace tinyrpc