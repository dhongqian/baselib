/*
 * @Author: your name
 * @Date: 2021-05-28 16:46:15
 * @LastEditTime: 2021-05-28 18:04:21
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \baselib\include\netwrapper\IClient.h
 */
#pragma once
#ifndef __HQ_NET_WRAPPER_ICLIENT_H__

#include <string>
#include <memory>
#include <iostream>
#include "baselib/Utility.h"
#include "netwrapper/HostInfo.h"

namespace hq {

class IClientCallback;
class BufferUtility;
class ISocketMgr;
class baselib_declspec IClient {
public:
    IClient() = default;
    virtual ~IClient() {};

    friend class ISocketMgr;

    typedef std::shared_ptr<IClientCallback>   IClientCallbackPtr;
    typedef std::shared_ptr<BufferUtility>     BufferUtilityPtr;

    virtual int setCallback(IClientCallbackPtr callback_ptr) = 0;

    virtual int send(std::shared_ptr<BufferUtility> buffer_ptr) =0;
    virtual int read(std::shared_ptr<BufferUtility> buffer_ptr) = 0;
    virtual int send(HostInfo &host_info, std::shared_ptr<BufferUtility> buffer_ptr) = 0;
    virtual int read(HostInfo &host_info, std::shared_ptr<BufferUtility> buffer_ptr) = 0;

    virtual void handleRead(const HostInfo &remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size) = 0;
    virtual void handleSend(const HostInfo &remote_host_info, std::size_t size) = 0;
    virtual void handleError(const int errror_code) = 0;

    virtual HostInfo localHostInfo() = 0;
    virtual HostInfo remoteHostInfo() = 0;

protected:
    virtual uint64_t getClientID() = 0;
};

typedef std::shared_ptr<IClient>  IClientPtr;

};

#endif ///< __HQ_NET_WRAPPER_ICLIENT_H__
