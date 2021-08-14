#pragma once
#ifndef __HQ_NET_WRAPPER_ISOCKET_MGR_H__

#include <memory>
#include "baselib/Utility.h"
#include "HostInfo.h"
#include "netwrapper/ITimerCallback.h"

namespace hq {

class IClientCallback;
class IClient;
class IServerCallback;

class baselib_declspec ISocketMgr{
public:
    ISocketMgr() = default;
    virtual ~ISocketMgr() {};

    typedef std::shared_ptr<IClientCallback>  IClientCallbackPtr;
    typedef std::shared_ptr<IClient>          IClientPtr;
    typedef std::shared_ptr<IServerCallback>  IServerCallbackPtr;

    virtual int start(const int pool_size = 0) = 0;
    virtual int stop() = 0;

    virtual IClientPtr startUdpClient(std::shared_ptr<IClientCallback> callback_ptr) = 0;
    virtual IClientPtr startUdpServer(const HostInfo &host_info, std::shared_ptr<IClientCallback> callback_ptr) = 0;
    virtual int stopUdpServer(std::shared_ptr<IClient> client_ptr) = 0;
    virtual IClientPtr startTcpClient(const HostInfo& host_info, std::shared_ptr<IClientCallback> callback_ptr) = 0;
    virtual int startTcpServer(const HostInfo& host_info, std::shared_ptr<IServerCallback> callback_ptr) = 0;
    virtual int stopTcpServer(const uint64_t id = 0) = 0;
    virtual int stopClient(const uint64_t client_id) = 0;

    virtual int startTimer(const int seconds, const int useconds, ITimerCallbackPtr callback_ptr) = 0;
    virtual int stopTimer(const int timer_id) = 0;
};

typedef std::shared_ptr<ISocketMgr>   ISocketMgrPtr;

baselib_declspec ISocketMgrPtr createSocketMgr();

}

#endif ///< __HQ_NET_WRAPPER_ISOCKET_MGR_H__
