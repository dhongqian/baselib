/*
 * @Author: your name
 * @Date: 2021-05-26 19:21:03
 * @LastEditTime: 2021-05-28 18:05:26
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \baselib\src\netwrapper\ClientCallback.h
 */
#pragma once
#ifndef __HQ_NET_WRAPPER_CLIENT_CALLBACK_H__

#include <string>
#include <memory>
#include "netwrapper/IClientCallback.h"
#include "baselib/BufferUtility.h"

namespace hq {

class baselib_declspec ClientCallback: public IClientCallback {
public:
    ClientCallback();
    virtual ~ClientCallback();

    virtual void handleRead(const HostInfo &remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size) = 0;
    virtual void handleSend(const HostInfo &remote_host_info, std::size_t size) = 0;
    virtual void handleError(const int errror_code) = 0;
};

typedef std::shared_ptr<ClientCallback>  ClientCallbackPtr;

};

#endif ///< __HQ_NET_WRAPPER_CLIENT_CALLBACK_H__
