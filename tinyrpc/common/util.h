#pragma once

#include <sys/types.h>
#include <unistd.h>

namespace tinyrpc {

pid_t getPid();

pid_t getThreadId();

int64_t getCurrentMs();

}