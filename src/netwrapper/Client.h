/*
 * @Author: your name
 * @Date: 2021-05-28 16:46:43
 * @LastEditTime: 2021-05-28 18:04:41
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \baselib\src\netwrapper\Client.h
 */
#pragma once
#ifndef __HQ_NET_WRAPPER_CLIENT_H__

#include <string>
#include <memory>
#include <iostream>
#include "ClientCallback.h"
#include "netwrapper/IClient.h"

namespace hq {

struct SocketData;
class baselib_declspec Client: public IClient {
public:
    Client(std::shared_ptr<SocketMgr> socket_mgr_ptr, const uint64_t client_id, const evutil_socket_t fd=-1);
    virtual ~Client();

    friend class SocketMgr;

    virtual int setCallback(IClientCallbackPtr callback_ptr);
    //virtual int setClientID(const HostInfo &host_info, IClientCallbackPtr callback_ptr, void *param=NULL);
    //virtual int stop();

    virtual int send(std::shared_ptr<BufferUtility> buffer_ptr);
    virtual int read(std::shared_ptr<BufferUtility> buffer_ptr);
    virtual int send(HostInfo& host_info, std::shared_ptr<BufferUtility> buffer_ptr);
    virtual int read(HostInfo& host_info, std::shared_ptr<BufferUtility> buffer_ptr);

    virtual void handleRead(const HostInfo &remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size);
    virtual void handleSend(const HostInfo &remote_host_info, std::size_t size);
    virtual void handleError(const int errror_code);
    virtual HostInfo localHostInfo();
    virtual HostInfo remoteHostInfo();

    //int start(IClientCallbackPtr callback_ptr, SocketDataPtr socket_data_ptr);

protected:
    virtual uint64_t getClientID();

protected:
    BufferUtilityPtr                 read_buffer_ptr_;
    IClientCallbackPtr               callback_ptr_;
    uint64_t                         client_id_;
    std::weak_ptr<SocketMgr>         socket_mgr_ptr_;
    const evutil_socket_t    socket_fd_;
    //std::weak_ptr<SocketData> socket_data_ptr_;
};
};

#endif ///< __HQ_NET_WRAPPER_CLIENT_H__
