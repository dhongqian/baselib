/*
 * @Author: your name
 * @Date: 2021-05-26 19:21:03
 * @LastEditTime: 2021-05-28 18:05:26
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \baselib\src\netwrapper\ClientCallback.h
 */
#pragma once
#ifndef __HQ_MANAGER_CLIENT_CALLBACK_H__

#include <string>
#include <memory>
#include "netwrapper/IClientCallback.h"
#include "baselib/BufferUtility.h"

namespace hq {

class /*baselib_declspec*/ ManagerClientCallback: public IClientCallback {
public:
    ManagerClientCallback();
    virtual ~ManagerClientCallback();

    virtual void handleRead(const HostInfo &remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size);
    virtual void handleSend(const HostInfo &remote_host_info, std::size_t size);
    virtual void handleError(const int errror_code);
};

typedef std::shared_ptr<ManagerClientCallback>  ManagerClientCallbackPtr;

};

#endif ///< __HQ_MANAGER_CLIENT_CALLBACK_H__
