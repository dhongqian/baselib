#pragma once
#ifndef __HQ_NET_WRAPPER_ISERVER_CALLBACK_H__

#include <memory>
#include "baselib/Utility.h"

namespace hq {

class IClient;
class baselib_declspec IServerCallback {
public:
    IServerCallback() = default;
    virtual ~IServerCallback() {};

    virtual void handleAcceptor(std::shared_ptr<IClient> client_ptr) = 0;
    virtual void handleError(const int error_code) = 0;
};

typedef std::shared_ptr<IServerCallback>  IServerCallbackPtr;

}

#endif  ///< __HQ_NET_WRAPPER_ISERVER_CALLBACK_H__
