#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>

namespace tinyrpc {

class NetAddr {
public:
    using s_ptr = std::shared_ptr<NetAddr>;

    virtual sockaddr* getSockAddr() = 0;

    virtual socklen_t getSockLen() = 0;

    virtual int getFamily() = 0;

    virtual std::string toString() = 0;

    virtual bool checkValid() = 0;

};

class IPNetAddr : public NetAddr {
public:
    IPNetAddr(const std::string& ip, uint16_t port);

    IPNetAddr(const std::string& addr);

    IPNetAddr(sockaddr_in addr);

    sockaddr* getSockAddr();

    socklen_t getSockLen();

    int getFamily();
    
    std::string toString();

    bool checkValid();

public:
    static bool CheckValid(const std::string& addr);

private:
    std::string ip_;
    uint16_t  port_ {0};
    sockaddr_in addr_;

};

}