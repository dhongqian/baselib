/*
 * @Author: your name
 * @Date: 2021-05-26 19:21:03
 * @LastEditTime: 2021-05-28 19:11:13
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \baselib\src\netwrapper\SocketMgr.h
 */
#pragma once

#include <netwrapper/Utility.h>
#include <memory>
#include <thread>
#include <string>

namespace hq {

namespace net {

void setThreadName(const std::string& thread_name) {
#ifndef _WIN32
    if(!thread->m_threadName.empty()) {
#ifdef __APPLE__
        pthread_setname_np(thread->m_threadName.c_str());
#elif defined(__linux__)
        prctl(PR_SET_NAME, thread->m_threadName.c_str());
#endif
    }
#endif
}

struct sockaddr_in getSockAddrFromHostInfo(const HostInfo& host_info) {
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_addr.s_addr = host_info.getIP();
    sin.sin_family = AF_INET;
    sin.sin_port = htons(host_info.getPort());

    return sin;
}

evutil_socket_t createNoBlockingSocket(const int type) {
    int fd = ::socket(AF_INET, type/*SOCK_DGRAM*/, 0);
    if(fd < 0) {
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " create socket failed.");
        return fd;
    }

    int ret = evutil_make_socket_nonblocking(fd);
    if(ret < 0) {
        evutil_closesocket(fd);
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " set socket noblock failed.");
        return -1;
    }
    return fd;
}

evutil_socket_t createBindNoBlockingSocket(const HostInfo& host_info, const int type) {
    int fd = createNoBlockingSocket(type);
    if(fd < 0) {
        return -1;
    }

    struct sockaddr_in sin = net::getSockAddrFromHostInfo(host_info);
    int ret = ::bind(fd, (struct sockaddr*)&sin, sizeof(sin));
    if(ret < 0) {
        evutil_closesocket(fd);
        LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("netwrapper"), " bind socket failed.");
        return ret;
    }
    return fd;
}

#ifdef WIN32
inline int win_inet_pton(int af, const char* src, void* dst) {
    struct sockaddr_storage ss;
    int size = sizeof(ss);
    char src_copy[INET6_ADDRSTRLEN + 1] = {0};

    ZeroMemory(&ss, sizeof(ss));
    /* stupid non-const API */
    int src_len = strlen(src);
    if(src_len > (INET6_ADDRSTRLEN - 1)) {
        return 0;
    }
    memcpy(src_copy, src, src_len);
    src_copy[INET6_ADDRSTRLEN] = 0;

    if(WSAStringToAddressA(src_copy, af, NULL, (struct sockaddr*)&ss, &size) == 0) {
        switch(af) {
            case AF_INET:
                *(struct in_addr*)dst = ((struct sockaddr_in*)&ss)->sin_addr;
                return 1;
            case AF_INET6:
                *(struct in6_addr*)dst = ((struct sockaddr_in6*)&ss)->sin6_addr;
                return 1;
        }
    }
    return 0;
}

inline const char* win_inet_ntop(int af, const void* src, char* dst, socklen_t size) {
    struct sockaddr_storage ss;
    unsigned long s = size;

    ZeroMemory(&ss, sizeof(ss));
    ss.ss_family = af;

    switch(af) {
        case AF_INET:
            ((struct sockaddr_in*)&ss)->sin_addr = *(struct in_addr*)src;
            break;
        case AF_INET6:
            ((struct sockaddr_in6*)&ss)->sin6_addr = *(struct in6_addr*)src;
            break;
        default:
            return NULL;
    }
    /* cannot direclty use &size because of strict aliasing rules */
    return (WSAAddressToStringA((struct sockaddr*)&ss, sizeof(ss), NULL, dst, &s) == 0) ?
        dst : NULL;
}

#endif

}

}

