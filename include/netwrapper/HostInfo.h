#ifndef _HQ_BASELIB_HOST_INFO_H_
#define _HQ_BASELIB_HOST_INFO_H_

#include <string>
#include <algorithm>
#include "baselib/Utility.h"

namespace hq{

/// @brief for ipv4 address and port
class baselib_declspec HostInfo {
public:
    HostInfo();
    HostInfo(const std::string &ip, const uint16_t port);
    HostInfo(const uint32_t ip, const uint16_t port);
    HostInfo(const struct sockaddr_in* addr_inet_v4);
    ~HostInfo();

    uint32_t getIP() const;
    const std::string getIPString();
    uint16_t getPort() const;
    bool isValid() const;

    void set(const uint32_t ip, uint16_t port);

    HostInfo(const HostInfo& host_info);
    HostInfo& operator=(const HostInfo& host_info);
    bool operator==(const HostInfo& host_info);
    bool operator<(const HostInfo& host_info);
    bool operator<=(const HostInfo& host_info);
    bool operator>(const HostInfo& host_info);
    bool operator>=(const HostInfo& host_info);
    bool operator!=(const HostInfo& host_info);

protected:
    uint32_t     ip_;
    uint16_t     port_;
};

}

#endif ///< _HQ_BASELIB_HOST_INFO_H_
