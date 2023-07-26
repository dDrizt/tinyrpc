#include <pthread.h>
#include "tinyrpc/common/log.h"
#include "tinyrpc/common/config.h"

int main() {
    tinyrpc::Config::SetGlobalConfigPath("/home/kiril/Dev/tinyrpc/conf/tinyrpc.xml");

    tinyrpc::Logger::InitGlobalLogger();

    pthread_t thread;
    pthread_create(&thread, nullptr, [](void*) -> void* {
        for (int i = 0; i < 10; ++i) {
            // LOG_DEBUG("debug this is thread in %s", "fun");
            DEBUGLOG("test debug log %s", "22");
        }
        return nullptr;
    }, nullptr);

    for (int i = 0; i < 10; ++i) {
        // LOG_DEBUG("debug this is main in %s", "11");
        DEBUGLOG("test debug log %s", "11");
    }

    pthread_join(thread, nullptr);

    return 0;
}


