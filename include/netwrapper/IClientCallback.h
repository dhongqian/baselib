/*
 * @Author: your name
 * @Date: 2021-05-26 19:20:01
 * @LastEditTime: 2021-05-28 18:05:16
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \baselib\include\netwrapper\IClientCallback.h
 */
#pragma once
#ifndef __HQ_NET_WRAPPER_ICLIENT_CALLBACK_H__

#include <string>
#include <memory>
#include "baselib/Utility.h"
#include "netwrapper/HostInfo.h"
#include "baselib/BufferUtility.h"

namespace hq {

class BufferUtility;
class baselib_declspec IClientCallback {
public:
    IClientCallback() = default;
    virtual ~IClientCallback() {};

    virtual void handleRead(const HostInfo &remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size) = 0;
    virtual void handleSend(const HostInfo &remote_host_info, std::size_t size) = 0;
    virtual void handleError(const int errror_code) = 0;
};

typedef std::shared_ptr<IClientCallback>  IClientCallbackPtr;

};

#endif ///< __HQ_NET_WRAPPER_ICLIENT_CALLBACK_H__
