﻿/*
 * @Author: your name
 * @Date: 2021-05-26 19:21:03
 * @LastEditTime: 2021-05-28 18:06:18
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \baselib\src\netwrapper\ClientCallback.cpp
 */
#include "ClientCallback.h"

namespace hq {

ClientCallback::ClientCallback() {
}

ClientCallback::~ClientCallback() {
}

void ClientCallback::handleRead(const HostInfo &remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size) {
    
}

void ClientCallback::handleSend(const HostInfo &remote_host_info, std::size_t size) {

}

void ClientCallback::handleError(const int errror_code) {

}

}
