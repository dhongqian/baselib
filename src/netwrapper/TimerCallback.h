/*
 * @Author: your name
 * @Date: 2021-05-26 19:21:03
 * @LastEditTime: 2021-05-28 18:05:26
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \baselib\src\netwrapper\ClientCallback.h
 */
#pragma once

#include <string>
#include <memory>
#include "netwrapper/IClientCallback.h"
#include "baselib/BufferUtility.h"

namespace hq {

class baselib_declspec ClientCallback: public IClientCallback {
public:
    ClientCallback();
    virtual ~ClientCallback();

    virtual void handle_read(const HostInfo &remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size) = 0;
    virtual void handle_send(const HostInfo &remote_host_info, std::size_t size) = 0;
    virtual void handle_error(const int errror_code) = 0;
};

typedef std::shared_ptr<ClientCallback>  ClientCallbackPtr;

};

