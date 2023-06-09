#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include "tinyrpc/common/util.h"

namespace tinyrpc {

static int g_pid = 0;

static thread_local int g_thread_id = 0;

pid_t getPid() {
    if (g_pid == 0) {
        g_pid = getpid();
    }

    return g_pid;
}

pid_t getThreadId() {
    if (g_thread_id == 0) {
        g_thread_id = syscall(SYS_gettid);
    }

    return g_thread_id;

}

int64_t getCurrentMs() {
    struct timeval now_time;
    gettimeofday(&now_time, nullptr);

    return now_time.tv_sec * 1000 + now_time.tv_usec / 1000;
}


}