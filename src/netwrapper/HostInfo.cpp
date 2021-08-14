#include "netwrapper/HostInfo.h"
#include <vector>

namespace hq{

HostInfo::HostInfo(): ip_(0), port_(0) {

}

HostInfo::HostInfo(const std::string& ip, const uint16_t port): port_(port) {
    ip_ = inet_addr(ip.c_str());
}

HostInfo::HostInfo(const uint32_t ip, const uint16_t port):ip_(ip), port_(port) {

}

HostInfo::HostInfo(const struct sockaddr_in* addr_inet_v4) : ip_(INADDR_ANY), port_(0) {
    ip_ = addr_inet_v4->sin_addr.s_addr;
    port_ = addr_inet_v4->sin_port;
}

HostInfo::~HostInfo() {

}

bool HostInfo::isValid() const {
    return (port_ != 0 && ip_ != INADDR_NONE);
}

uint32_t HostInfo::getIP() const {
    return ip_;
}

const std::string HostInfo::getIPString() {
    in_addr addr;
    addr.S_un.S_addr = ip_;
    return inet_ntoa(addr);
}

uint16_t HostInfo::getPort() const {
    return port_;
}

void HostInfo::set(const uint32_t ip, uint16_t port) {
    ip_ = ip;
    port_ = port;
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
    port_ = host_info.port_;
    ip_ = host_info.ip_;
}

HostInfo& HostInfo::operator=(const HostInfo& host_info) {
    if(&host_info == this) return *this;
    ip_ = host_info.ip_;
    port_ = host_info.port_;
    return *this;
}

bool HostInfo::operator==(const HostInfo& host_info) {
    return(ip_ == host_info.ip_ && port_ == host_info.port_);
}

bool HostInfo::operator<(const HostInfo& host_info) {
    if(ip_ == host_info.ip_) return (port_ < host_info.port_);
    return ip_ < host_info.ip_;
}

bool HostInfo::operator<=(const HostInfo& host_info) {
    if(ip_ == host_info.ip_) return (port_ <= host_info.port_);
    return ip_ < host_info.ip_;
}

bool HostInfo::operator>(const HostInfo& host_info) {
    if(ip_ == host_info.ip_) return (port_ > host_info.port_);
    return ip_ > host_info.ip_;
}

bool HostInfo::operator>=(const HostInfo& host_info) {
    if(ip_ == host_info.ip_) return (port_ >= host_info.port_);
    return ip_ > host_info.ip_;
}

bool HostInfo::operator!=(const HostInfo& host_info) {
    return (ip_ != host_info.ip_ || port_ != host_info.port_);
}

}
