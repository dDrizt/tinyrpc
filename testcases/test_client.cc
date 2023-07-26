#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include "tinyrpc/common/log.h"
#include "tinyrpc/common/config.h"

#define BUFF_SIZE 128

void test_connect() {
    // 调用 connect 连接 server
    // write 一个字符串
    // 等待 read 返回结果
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        ERRORLOG("Invalid fd %d", fd);
        exit(0);
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    int ret = connect(fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

    DEBUGLOG("connnect success");

    std::string msg = "Hello TinyRpc";

    ret = write(fd, msg.c_str(), msg.length());
    DEBUGLOG("Success write %d bytes, [%s]", ret, msg.c_str());

    char buf[BUFF_SIZE];
    ret = read(fd, buf, BUFF_SIZE);
    DEBUGLOG("Success read %d bytes, [%s]", ret, std::string(buf).c_str());

}

int main() {
    tinyrpc::Config::SetGlobalConfigPath("../conf/tinyrpc.xml");

    tinyrpc::Logger::InitGlobalLogger();

    test_connect();

    return 0;
}