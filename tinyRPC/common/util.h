#ifndef TINYRPC_COMMON_UTIL_H
#define TINYRPC_COMMON_UTIL_H

#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

namespace tinyRPC {

pid_t getThreadId();

pid_t getPid();

};

#endif