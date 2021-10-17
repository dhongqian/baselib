#pragma once
#ifndef __HQ_MANAGER_SERVER_CALLBACK_H__

#include "netwrapper/IServerCallback.h"

namespace hq {

class ManagerServerCallback: public IServerCallback {
public:
    ManagerServerCallback();
    virtual ~ManagerServerCallback();

    virtual void handleAcceptor(std::shared_ptr<IClient> client_ptr);
    virtual void handleError(const int error_code);

protected:
    std::map<uint64_t, std::shared_ptr<IClient>>   client_map_;
};

typedef std::shared_ptr<ManagerServerCallback>  ManagerServerCallbackPtr;

}

#endif ///< __HQ_MANAGER_SERVER_CALLBACK_H__
