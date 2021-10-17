#pragma once
#ifndef __HQ_NET_WRAPPER_UTILITY_H__

#include <memory>
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
#include "baselib/Utility.h"
#include "HostInfo.h"

namespace hq {

namespace net {

void setThreadName(const std::string& thread_name);
struct sockaddr_in getSockAddrFromHostInfo(const HostInfo& host_info);
evutil_socket_t createNoBlockingSocket(const int type);
evutil_socket_t createBindNoBlockingSocket(const HostInfo& host_info, const int type);

}

}

#endif ///< __HQ_NET_WRAPPER_UTILITY_H__
