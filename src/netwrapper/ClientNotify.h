#pragma once
#ifndef __HQ_NET_WRAPPER_CLIENT_NOTIFY_H__

#include <string>
#include <memory>
#include "netwrapper/IClientNotify.h"

namespace hq {

class ClientNotify: public IClientNotify {
public:
    ClientNotify () = default;
    virtual ~ClientNotify() {};

    virtual void notify(const uint64_t notify_id);
};

typedef std::shared_ptr<ClientNotify>  ClientNotifyPtr;

};

#endif ///< __HQ_NET_WRAPPER_CLIENT_NOTIFY_H__
