#ifndef _HQ_BASELIB_HOST_INFO_H_
#define _HQ_BASELIB_HOST_INFO_H_

extern "C" {
#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/thread.h>

#ifndef WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#else
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib,"Ws2_32.lib ")
#include <Iphlpapi.h>
#pragma  comment(lib, "Iphlpapi.lib")
#include <io.h>
#endif
}

#include <string>
#include <algorithm>
#include "baselib/Utility.h"

namespace hq{

class SocketAddr;
typedef std::shared_ptr<SocketAddr> SocketAddrPtr;

/// @brief for ipv4 address and port
class baselib_declspec HostInfo {
public:
    HostInfo();
    HostInfo(const std::string &ip, const uint16_t port);
    HostInfo(const uint32_t ip, const uint16_t port);
    HostInfo(const struct sockaddr_in* addr_inet_v4);
    HostInfo(const struct sockaddr_in6* addr_inet_v6);
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
    /*uint32_t     ip_;
    uint16_t     port_;*/
    SocketAddr* socket_addr_;
};

}

#endif ///< _HQ_BASELIB_HOST_INFO_H_
