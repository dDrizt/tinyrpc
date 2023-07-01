#include "tinyrpc/common/log.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include <string.h>

namespace tinyrpc {

IPNetAddr::IPNetAddr(const std::string& ip, uint16_t port) : ip_(ip), port_(port) {
    memset(&addr_, 0, sizeof(addr_));

    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip_.c_str());
    addr_.sin_port = htons(port_);
}

IPNetAddr::IPNetAddr(const std::string& addr) {
    size_t idx = addr.find_first_of(":");
    if (idx == addr.npos) {
        ERRORLOG("Invalid IPv4 addr %s", addr.c_str());
        return;
    }

    ip_ = addr.substr(0, idx);
    port_ = std::atoi(addr.substr(idx + 1, addr.size() - idx - 1).c_str());

    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip_.c_str());
    addr_.sin_port = htons(port_);
}

IPNetAddr::IPNetAddr(sockaddr_in addr) : addr_(addr) {
    ip_ = static_cast<std::string>(inet_ntoa(addr_.sin_addr));
    port_ = ntohs(addr_.sin_port);
}

sockaddr* IPNetAddr::getSockAddr() {
    return reinterpret_cast<sockaddr*>(&addr_);
}

socklen_t IPNetAddr::getSockLen() const {
    return sizeof(addr_);
}

int IPNetAddr::getFamily() const {
    return AF_INET;
}

std::string IPNetAddr::toString() {
    std::string tmp;
    tmp = ip_ + ":" + std::to_string(port_);
    return tmp;
}

bool IPNetAddr::checkValid() {
    if (ip_.empty())
        return false;
    
    if (port_ < 0 || port_ > 65536)
        return false;

    if (inet_addr(ip_.c_str()) == INADDR_NONE)
        return false;
    
    return true;
}


}