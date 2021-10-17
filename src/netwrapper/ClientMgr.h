/*
 * @Author: your name
 * @Date: 2021-05-28 16:46:43
 * @LastEditTime: 2021-05-28 18:04:41
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \baselib\src\netwrapper\Client.h
 */
#pragma once
#ifndef __HQ_NET_WRAPPER_CLIENT_MANAGER_H__

#include <string>
#include <memory>
#include <iostream>
#include "ClientCallback.h"
#include "netwrapper/IClient.h"

namespace hq {

class ClientMgr;
struct SocketData {
    uint64_t     id;
    evutil_socket_t fd;
    event* normal_event;
    bufferevent* buffer_event;
    ClientMgr *client_mgr;
    HostInfo     host_info;
    std::weak_ptr<IClient> client_ptr;
    BufferUtilityPtr  read_buff_ptr;

    SocketData() {
        memset(this, 0, offsetof(SocketData, host_info));
        read_buff_ptr = std::make_shared<BufferUtility>();
    }

    void reset();
};
typedef std::shared_ptr<SocketData>  SocketDataPtr;

class SocketMgr;
class baselib_declspec ClientMgr {
public:
    ClientMgr(std::shared_ptr<SocketMgr> socket_mgr_ptr);
    virtual ~ClientMgr();

    IClientPtr startUdpClient(std::shared_ptr<IClientCallback> callback_ptr);
    IClientPtr startUdpServer(const HostInfo& host_info, std::shared_ptr<IClientCallback> callback_ptr);
    IClientPtr startTcpClient(const HostInfo& host_info, std::shared_ptr<IClientCallback> callback_ptr);
    IClientPtr startTcpClient(evutil_socket_t fd, std::shared_ptr<IClientCallback> callback_ptr);
    virtual int stopAllClient();
    virtual int stopClient(const uint64_t client_id);

public:
    int send(const uint64_t client_id, std::shared_ptr<BufferUtility> buffer_ptr);
    int send(const uint64_t client_id, HostInfo& host_info, std::shared_ptr<BufferUtility> buffer_ptr);
    void handleError(SocketData* socket_data, const int error_code);

protected:
    int add2socketDataMap(SocketData* socket_data);
    int closeSocketData(SocketData*& socket_data);

    evutil_socket_t getFDByClientID(const uint64_t client_id);
    int closeClientByClientID(const uint64_t client_id);

protected:
    std::weak_ptr<SocketMgr>         socket_mgr_ptr_;
    std::mutex socket_data_map_mutex_;
    std::map<uint64_t, SocketData*>    socket_data_map_;
};
};

#endif ///< __HQ_NET_WRAPPER_CLIENT_MANAGER_H__
