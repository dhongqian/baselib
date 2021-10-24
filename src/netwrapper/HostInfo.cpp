#include "netwrapper/HostInfo.h"
#include <vector>
#include <cassert>
#include <netwrapper/Utility.h>

namespace hq{

//////////////// SocketAddr//////////////////////////////
class SocketAddr {
public:
    SocketAddr(bool is_v6=false): is_v6_(is_v6) {}
    virtual ~SocketAddr() {}

    virtual std::string str() = 0;
    virtual std::string getIP() = 0;
    virtual uint16_t getPort() = 0;
    virtual SocketAddr* clone() = 0;
    bool isV6() { return is_v6_; };
    virtual bool isEqual(SocketAddr *) = 0;
    virtual bool isGreater(SocketAddr*) = 0;
    virtual bool isLess(SocketAddr*) = 0;

private:
    const bool is_v6_;
};


//////////////// SocketAddrv4//////////////////////////////
class SocketAddrv4: public SocketAddr {
public:
    SocketAddrv4(const uint32_t ip, uint16_t port): SocketAddr(false), ip_(ip), port_(port) {}
    SocketAddrv4(const std::string& ip, const uint16_t port): SocketAddr(false), port_(port) {
        ::inet_pton(AF_INET, ip.c_str(), &ip_);
    }
    virtual ~SocketAddrv4() {}

    SocketAddrv4(const SocketAddrv4& socket_addr) {
        if(this == &socket_addr) return;
        this->ip_ = socket_addr.ip_;
        this->port_ = socket_addr.port_;
    }

    SocketAddrv4& operator=(const SocketAddrv4& socket_addr) {
        if(this == &socket_addr) return *this;
        this->ip_ = socket_addr.ip_;
        this->port_ = socket_addr.port_;
        return (*this);
    }

    virtual std::string str() {
        std::ostringstream oss;
        oss << getIP() << ":" << port_;
        return oss.str();
    }

    virtual std::string getIP() {
#define INET6_ADDRSTRLEN 65
        char tempip[INET6_ADDRSTRLEN] = {0};
        ::inet_ntop(AF_INET, &ip_, tempip, INET6_ADDRSTRLEN);

        return tempip;
    }

    virtual uint16_t getPort() {
        return port_;
    }

    virtual SocketAddr* clone() {
        auto addr_ptr = new SocketAddrv4(this->ip_, this->port_);
        return addr_ptr;
    }

    virtual bool isEqual(SocketAddr* socket_addr) {
        if(nullptr == socket_addr) return false;
        if(socket_addr->isV6()) return false;

        SocketAddrv4* addr_ptr = dynamic_cast<SocketAddrv4*>(socket_addr);
        if(nullptr == addr_ptr) return false;
        return ((*this) == (*addr_ptr));
    }

    virtual bool isGreater(SocketAddr* socket_addr) {
        if(nullptr == socket_addr) return false;
        if(socket_addr->isV6()) return false;

        SocketAddrv4* addr_ptr = dynamic_cast<SocketAddrv4*>(socket_addr);
        if(nullptr == addr_ptr) return false;
        return ((*this) > (*addr_ptr));
    }

    virtual bool isLess(SocketAddr* socket_addr) {
        if(nullptr == socket_addr) return false;
        if(socket_addr->isV6()) return true;

        SocketAddrv4* addr_ptr = dynamic_cast<SocketAddrv4*>(socket_addr);
        if(nullptr == addr_ptr) return false;
        return ((*this) < (*addr_ptr));
    }

    //virtual bool isV6();
    virtual bool operator==(const SocketAddrv4 &socket_addr) {
        return (this->ip_ == socket_addr.ip_ || this->port_ == socket_addr.port_);
    }

    virtual bool operator>(const SocketAddrv4& socket_addr) {
        if(this->ip_ < socket_addr.ip_) return false;
        if(this->ip_ > socket_addr.ip_) return true;
        return (this->port_ > socket_addr.port_);
    }

    virtual bool operator>=(const SocketAddrv4& socket_addr) {
        return ((*this > socket_addr) || (*this == socket_addr));
    }

    virtual bool operator<(const SocketAddrv4& socket_addr) {
        return !(*this >= socket_addr);
    }

    virtual bool operator<=(const SocketAddrv4& socket_addr) {
        return !(*this > socket_addr);
    }

    virtual bool operator!=(const SocketAddrv4& socket_addr) {
        return (this->ip_ != socket_addr.ip_ || this->port_ != socket_addr.port_);
    }

protected:
    uint32_t  ip_;
    uint16_t  port_;
};

////////////////SocketAddrv6//////////////////////
class SocketAddrv6: public SocketAddr {
public:
    SocketAddrv6(): SocketAddr(true), port_(0) {
        memset(ip_, 0, sizeof(ip_));
        
    }

    SocketAddrv6(const std::string &ip, const uint16_t port): SocketAddr(true), port_(port) {
        //memcpy((char*)ip_, (char*)ip, 16);
#ifdef WIN32
        ::inet_pton(AF_INET6, ip.c_str(), ip_);
#else
        ::inet_pton(AF_INET6, ip.c_str(), ip_);
#endif
    }

    SocketAddrv6(const uint8_t *ip, const uint16_t port): SocketAddr(true), port_(port) {
        memcpy((char*)ip_, (char*)ip, 16);
    }

    virtual ~SocketAddrv6() {}

    SocketAddrv6(const SocketAddrv6& socket_addr) {
        if(this == &socket_addr) return;
        memcpy(this->ip_, socket_addr.ip_, 16);
        this->port_ = socket_addr.port_;
    }

    SocketAddrv6& operator=(const SocketAddrv6& socket_addr) {
        if(this == &socket_addr) return *this;
        memcpy(this->ip_, socket_addr.ip_, 16);
        this->port_ = socket_addr.port_;
        return (*this);
    }

    virtual std::string str() {
        std::ostringstream oss;
        oss << getIP() << ":" << port_;
        return oss.str();
    }

    virtual std::string getIP() {
#define INET6_ADDRSTRLEN 65
        char tempip[INET6_ADDRSTRLEN] = {0};
        ::inet_ntop(AF_INET6, ip_, tempip, INET6_ADDRSTRLEN);

        return tempip;
    }

    virtual uint16_t getPort() {
        return port_;
    }
    virtual SocketAddr* clone() {
        SocketAddr* addr_ptr = new SocketAddrv6(ip_, port_);
        return addr_ptr;
    }

//    virtual bool isV6() = 0;
    virtual bool isEqual(SocketAddr* socket_addr) {
        if(nullptr == socket_addr) return false;
        if(!socket_addr->isV6()) return true;

        SocketAddrv6* addr_ptr = dynamic_cast<SocketAddrv6*>(socket_addr);
        if(nullptr == addr_ptr) return false;
        return ((*this) == (*addr_ptr));
    }

    virtual bool isGreater(SocketAddr* socket_addr) {
        if(nullptr == socket_addr) return false;
        if(!socket_addr->isV6()) return true;

        SocketAddrv6* addr_ptr = dynamic_cast<SocketAddrv6*>(socket_addr);
        if(nullptr == addr_ptr) return false;
        return ((*this) > (*addr_ptr));
    }

    virtual bool isLess(SocketAddr* socket_addr) {
        if(nullptr == socket_addr) return false;
        if(!socket_addr->isV6()) return false;

        SocketAddrv6* addr_ptr = dynamic_cast<SocketAddrv6*>(socket_addr);
        if(nullptr == addr_ptr) return false;
        return ((*this) < (*addr_ptr));
    }

    virtual bool operator==(const SocketAddrv6& socket_addr) {
        char* src = (char*)this->ip_, *dst=(char*)socket_addr.ip_;
        return (0==std::strncmp(src, dst, 16) || this->port_ == socket_addr.port_);
    }

    virtual bool operator>(const SocketAddrv6& socket_addr) {
        char* src = (char*)this->ip_, * dst = (char*)socket_addr.ip_;
        return (0 < std::strncmp(src, dst, 16) || this->port_ > socket_addr.port_);
    }

    virtual bool operator>=(const SocketAddrv6& socket_addr) {
        return ((*this > socket_addr) || (*this == socket_addr));
    }

    virtual bool operator<(const SocketAddrv6& socket_addr) {
        return !(*this >= socket_addr);
    }

    virtual bool operator<=(const SocketAddrv6& socket_addr) {
        return !(*this > socket_addr);
    }

    virtual bool operator!=(const SocketAddrv6& socket_addr) {
        return (this->ip_ != socket_addr.ip_ || this->port_ != socket_addr.port_);
    }

protected:
    uint8_t  ip_[16];
    uint16_t  port_;
};
//////////////////////////////////////////////////

HostInfo::HostInfo()/*: ip_(0), port_(0)*/ {
    socket_addr_ = new SocketAddrv4(INADDR_ANY, 0);
    assert(nullptr != socket_addr_);
}

HostInfo::HostInfo(const std::string& ip, const uint16_t port)/*: port_(port)*/ {
    //ip_ = inet_addr(ip.c_str());
    if(std::string::npos != ip.find(".")) { /// ipv4
        socket_addr_ = new SocketAddrv4(ip, port);
    }
    else {
        socket_addr_ = new SocketAddrv6(ip, port);
    }
}

HostInfo::HostInfo(const uint32_t ip, const uint16_t port)/*:ip_(ip), port_(port)*/ {
    socket_addr_ = new SocketAddrv4(ip, port);
}

HostInfo::HostInfo(const struct sockaddr_in* addr_inet_v4)/* : ip_(INADDR_ANY), port_(0)*/ {
    //ip_ = addr_inet_v4->sin_addr.s_addr;
    //port_ = addr_inet_v4->sin_port;
    socket_addr_ = new SocketAddrv4(addr_inet_v4->sin_addr.s_addr, addr_inet_v4->sin_port);
}

HostInfo::HostInfo(const struct sockaddr_in6* addr_inet_v6) {
    socket_addr_ = new SocketAddrv6(addr_inet_v6->sin6_addr.u.Byte, addr_inet_v6->sin6_port);
}

HostInfo::~HostInfo() {
    if(nullptr != socket_addr_) {
        delete socket_addr_;
        socket_addr_ = nullptr;
    }
}

bool HostInfo::isValid() const {
    //return (port_ != 0 && ip_ != INADDR_NONE);
    return true;
}

uint32_t HostInfo::getIP() const {
    if(nullptr == socket_addr_ || socket_addr_->isV6()) return 0;
    int ip = 0;
    inet_pton(AF_INET, socket_addr_->getIP().c_str(), &ip);
    return ip;
}

const std::string HostInfo::getIPString() {
    //in_addr addr;
    //addr.S_un.S_addr = ip_;
    //return inet_ntoa(addr);
    if(nullptr == socket_addr_) return "";
    return socket_addr_->getIP();
}

uint16_t HostInfo::getPort() const {
    if(nullptr == socket_addr_) return 0;
    return socket_addr_->getPort();
}

void HostInfo::set(const uint32_t ip, uint16_t port) {
    /*ip_ = ip;
    port_ = port;*/
    if(nullptr != socket_addr_) {
        delete socket_addr_;
    }
    socket_addr_ = new SocketAddrv4(ip, port);
}

//::sockaddr_in HostInfo::getSockAddr() {
//    struct sockaddr_in sin;
//    memset(&sin, 0, sizeof(sin));
//    sin.sin_addr.s_addr = getIP();
//    sin.sin_family = AF_INET;
//    sin.sin_port = htons(getPort());
//
//    return sin;
//}

HostInfo::HostInfo(const HostInfo& host_info) {
    if(nullptr != socket_addr_) {
        delete socket_addr_;
        socket_addr_ = nullptr;
    }
    if(nullptr != host_info.socket_addr_) {
        socket_addr_ = host_info.socket_addr_->clone();
    }
}

HostInfo& HostInfo::operator=(const HostInfo& host_info) {
    if(&host_info == this) return *this;
    if(nullptr != socket_addr_) {
        delete socket_addr_;
        socket_addr_ = nullptr;
    }
    if(nullptr != host_info.socket_addr_) {
        socket_addr_ = host_info.socket_addr_->clone();
    }
    return *this;
}

bool HostInfo::operator==(const HostInfo& host_info) {
    if(nullptr == this->socket_addr_ && nullptr == host_info.socket_addr_) return true;
    if(nullptr == this->socket_addr_ || nullptr == host_info.socket_addr_) return false;
    return this->socket_addr_->isEqual(host_info.socket_addr_);
}

bool HostInfo::operator<(const HostInfo& host_info) {
    if(nullptr == this->socket_addr_ && nullptr == host_info.socket_addr_) return false;
    if(nullptr == this->socket_addr_ || nullptr == host_info.socket_addr_) return false;
    return this->socket_addr_->isLess(host_info.socket_addr_);
}

bool HostInfo::operator<=(const HostInfo& host_info) {
    if(nullptr == this->socket_addr_ && nullptr == host_info.socket_addr_) return true;
    if(nullptr == this->socket_addr_ || nullptr == host_info.socket_addr_) return false;
    return (this->socket_addr_->isLess(host_info.socket_addr_) || this->socket_addr_->isEqual(host_info.socket_addr_));
}

bool HostInfo::operator>(const HostInfo& host_info) {
    return !(*this <= host_info);
}

bool HostInfo::operator>=(const HostInfo& host_info) {
    return !(*this < host_info);
}

bool HostInfo::operator!=(const HostInfo& host_info) {
    return !(*this == host_info);
}

}
