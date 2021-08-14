#pragma once
#ifndef __HQ_NET_WRAPPER_SERVER_CALLBACK_H__

#include "netwrapper/IServerCallback.h"

namespace hq {

class ServerCallback: public IServerCallback {
public:
    ServerCallback();
    virtual ~ServerCallback();

    virtual void handleAcceptor(std::shared_ptr<IClient> client_ptr) = 0;
    virtual void handleError(const int error_code) = 0;
};

typedef std::shared_ptr<ServerCallback>  ServerCallbackPtr;

}

#endif ///< __HQ_NET_WRAPPER_SERVER_CALLBACK_H__
