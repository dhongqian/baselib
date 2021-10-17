/*
 * @Author: your name
 * @Date: 2021-05-28 16:46:43
 * @LastEditTime: 2021-05-28 18:04:41
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \baselib\src\netwrapper\Client.h
 */
#pragma once
#ifndef __HQ_NET_WRAPPER_SERVER_MANAGER_H__

#include <string>
#include <memory>
#include <iostream>
#include "ClientCallback.h"
#include "netwrapper/IClient.h"

namespace hq {

class ServerMgr;
struct ListenerData {
    uint64_t     id;
    struct evconnlistener* tcp_listener;
    ServerMgr *server_mgr;
    HostInfo     host_info;
    std::shared_ptr<IServerCallback>  server_callback;
};

class SocketMgr;
class baselib_declspec ServerMgr {
public:
    ServerMgr(std::shared_ptr<SocketMgr> socket_mgr_ptr);
    virtual ~ServerMgr();

    int startTcpServer(const HostInfo& host_info, std::shared_ptr<IServerCallback> callback_ptr);
    int stopTcpServer(const uint64_t id/*=0*/);
    int stopAllTcpServer();

    void handleAccept(struct evconnlistener* tcp_listener, evutil_socket_t fd, HostInfo& remote_host_info, void* ptr);

protected:
    void sendNotify();

protected:
    std::weak_ptr<SocketMgr>         socket_mgr_ptr_;
    std::mutex listener_data_map_mutex_;
    std::map<uint64_t, ListenerData*>  listener_data_map_;
};
};

#endif ///< __HQ_NET_WRAPPER_CLIENT_MANAGER_H__
