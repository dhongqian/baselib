#pragma once
#ifndef __HQ_NET_WRAPPER_UTILITY_H__

#include "HostInfo.h"
#include <memory>
#include "baselib/Utility.h"

namespace hq {

namespace net {

void setThreadName(const std::string& thread_name);
struct sockaddr_in getSockAddrFromHostInfo(const HostInfo& host_info);
evutil_socket_t createNoBlockingSocket(const int type);
evutil_socket_t createBindNoBlockingSocket(const HostInfo& host_info, const int type);

#ifdef WIN32
inline int win_inet_pton(int af, const char* src, void* dst);
inline const char* win_inet_ntop(int af, const void* src, char* dst, socklen_t size);
#endif

}

}

#endif ///< __HQ_NET_WRAPPER_UTILITY_H__
