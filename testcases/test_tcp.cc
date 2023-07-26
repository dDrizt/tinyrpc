#include "tinyrpc/common/log.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_server.h"

void test_tcp_server() {
    tinyrpc::IPNetAddr::s_ptr addr = std::make_shared<tinyrpc::IPNetAddr>("127.0.0.1", 12345);

    DEBUGLOG("create addr %s", addr->toString().c_str());

    tinyrpc::TcpServer tcp_server(addr);

    tcp_server.start();
}

int main() {
    tinyrpc::Config::SetGlobalConfigPath("../conf/tinyrpc.xml");
    tinyrpc::Logger::InitGlobalLogger();

    test_tcp_server();
}