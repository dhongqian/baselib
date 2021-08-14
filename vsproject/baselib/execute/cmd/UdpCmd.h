#pragma once
#include <string>
#include <atomic>
#include "baselib/Utility.h"
#include "netwrapper/HostInfo.h"
#include "netwrapper/IClient.h"
#include "netwrapper/IClientCallback.h"
#include "netwrapper/ITimerCallback.h"

namespace hq {

class UdpCmd: public ITimerCallback{
public:
    UdpCmd();
    virtual ~UdpCmd();

    void init(IClientPtr client_ptr);
    int start();
    int stop();

    virtual void handleRead(const HostInfo& remote_host_info, BufferUtilityPtr read_buffer_ptr, std::size_t size);
    virtual void handleSend(const HostInfo& remote_host_info, std::size_t size);
    virtual void handleError(const int errror_code);

    virtual void timerCallback(const int timer_id);

protected:
    void keepAlive();

protected:
    HostInfo remote_server_;
    bool is_start_;
    std::weak_ptr<IClient>  client_ptr_;
    std::atomic<unsigned int>           keep_alive_count_;
};

typedef std::shared_ptr<UdpCmd> UdpCmdPtr;

}