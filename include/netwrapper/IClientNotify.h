#pragma once
#ifndef __HQ_NET_WRAPPER_ICLIENT_NOTIFY_H__

#include <string>
#include <memory>
#include "baselib/Utility.h"

namespace hq {

class baselib_declspec IClientNotify {
public:
    IClientNotify () = default;
    virtual ~IClientNotify() {};

    virtual void notify(const uint64_t notify_id) = 0;
};

typedef std::shared_ptr<IClientNotify>  IClientNotifyPtr;

};

#endif ///< __HQ_NET_WRAPPER_ICLIENT_NOTIFY_H__
