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
}

}

